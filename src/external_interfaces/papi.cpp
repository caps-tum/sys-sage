#include "sys-sage.hpp"
#include <papi.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stddef.h>
#include <sched.h>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

// note: This macro produces timestamps using a monotonic clock.
//       While the timestamps within a single runtime of the program are
//       guaranteed to be monotonic, the clock does not guarantee to be
//       globally monotonic across multiple launches of the program and across
//       system boot. Thus, the timestamp does not uniquely identify a single
//       performance counter value.
#define TIME() ( std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() )

using namespace sys_sage;

static const std::string metricsKey ( "PAPIMetrics" );

struct PAPIMetrics {
  std::unordered_map<std::string,
                     std::vector<std::pair<unsigned long long, long long>>> counters;
};

static std::optional<unsigned int> GetCpuNumFromTid(unsigned long tid)
{
  static constexpr int hwThreadIdField = 39;

  std::ifstream procStat ("/proc/" + std::to_string(tid) + "/stat");
  if (!procStat.is_open())
    return std::nullopt;

  std::string line;
  if (!std::getline(procStat, line))
    return std::nullopt;

  procStat.close();

  size_t pos = line.find_last_of(')');
  if (pos == std::string::npos)
    return std::nullopt;

  // iterate until the white space before `hwThreadIdField` is found
  pos++;
  for (int field = 2; field < hwThreadIdField - 1; field++) {
    pos = line.find_first_of(' ', pos + 1);
    if (pos == std::string::npos)
      return std::nullopt;
  }

  size_t endPos = line.find_first_of(' ', pos + 1);
  if (endPos == std::string::npos)
    return std::nullopt;

  std::istringstream strStream ( line.substr(pos, endPos - pos) );
  unsigned int cpuNum;
  strStream >> cpuNum;

  return cpuNum;
}

static int GetCpuNum(int eventSet, unsigned int *cpuNum)
{
  int rval;
  PAPI_option_t opt;

  int state;
  rval = PAPI_state(eventSet, &state);
  if (rval != PAPI_OK)
    return rval;

  if (state & PAPI_CPU_ATTACHED) {
    opt.cpu.eventset = eventSet;
    opt.cpu.cpu_num = PAPI_NULL;
    rval = PAPI_get_opt(PAPI_CPU_ATTACH, &opt);
    if (rval < 0)
      return rval;

    *cpuNum = opt.cpu.cpu_num;
  } else if (state & PAPI_ATTACHED) {
    opt.attach.eventset = eventSet;
    opt.attach.tid = PAPI_NULL;
    rval = PAPI_get_opt(PAPI_ATTACH, &opt);
    if (rval < 0)
      return rval;

    std::optional<unsigned int> optCpuNum = GetCpuNumFromTid(opt.attach.tid);
    if (!optCpuNum)
      return PAPI_EINVAL;

    *cpuNum = *optCpuNum;
  } else {
    rval = sched_getcpu();
    if (rval < 0)
      return PAPI_ESYS;

    *cpuNum = rval;
  }

  return PAPI_OK;
}

static int GetEvents(int eventSet, std::unique_ptr<int[]> &events,
                     int *numEvents)
{
  int rval;

  rval = PAPI_num_events(eventSet);
  if (rval < 0)
    return rval;
  else if (rval == 0)
    return PAPI_EINVAL;
  *numEvents = rval;

  events = std::make_unique<int[]>(rval);
  rval = PAPI_list_events(eventSet, events.get(), numEvents);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

template <bool accumulate>
static int StoreCounters(const long long *counters, const int *events,
                         int numEvents, Thread *thread,
                         unsigned long long *outTimestamp)
{
  int rval;

  PAPIMetrics *metrics;
  auto metricsIt = thread->attrib.find(metricsKey);
  if (metricsIt == thread->attrib.end()) {
    metrics = new PAPIMetrics();
    thread->attrib[metricsKey] = reinterpret_cast<void *>( metrics );
  } else {
    metrics = reinterpret_cast<PAPIMetrics *>( metricsIt->second );
  }

  std::string buf (PAPI_MAX_STR_LEN, '\0');

  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf.data());
    if (rval != PAPI_OK)
      return rval;

    auto [it, _] = metrics->counters.try_emplace(buf);
    auto &readings = it->second;

    unsigned long long timestamp;
    if constexpr (accumulate) {
      if (readings.empty()) {
        // TODO: what if timestamps collide?
        timestamp = TIME();
        readings.emplace_back(timestamp, counters[i]);
      } else {
        // always accumulate on the last reading
        auto &pair = readings.back();
        timestamp = pair.first;
        pair.second += counters[i];
      }
    } else {
      // TODO: what if timestamps collide?
      timestamp = TIME();
      readings.emplace_back(timestamp, counters[i]);
    }

    if (outTimestamp)
      *outTimestamp = timestamp;
  }

  return PAPI_OK;
}

int sys_sage::PAPI_read(int eventSet, Component *root, Thread **outThread,
                        unsigned long long *outTimestamp)
{
  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  long long counters[numEvents];
  rval = ::PAPI_read(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  // TODO: make `GetSubcomponentById` take in an `unsigned int` instead of `int`
  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  if (outThread)
    *outThread = thread;
  rval = StoreCounters<false>(counters, events.get(), numEvents, thread,
                              outTimestamp);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

int sys_sage::PAPI_accum(int eventSet, Component *root, Thread **outThread,
                         unsigned long long *outTimestamp)
{
  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  long long counters[numEvents] = { 0 };
  rval = ::PAPI_accum(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  // TODO: make `GetSubcomponentById` take in an `unsigned int` instead of `int`
  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  if (outThread)
    *outThread = thread;
  rval = StoreCounters<true>(counters, events.get(), numEvents, thread,
                             outTimestamp);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

int sys_sage::PAPI_stop(int eventSet, Component *root, Thread **outThread,
                        unsigned long long *outTimestamp)
{
  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  long long counters[numEvents];
  rval = ::PAPI_stop(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  // TODO: make `GetSubcomponentById` take in an `unsigned int` instead of `int`
  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  if (outThread)
    *outThread = thread;
  rval = StoreCounters<false>(counters, events.get(), numEvents, thread,
                              outTimestamp);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

int sys_sage::PAPI_store(int eventSet, const long long *counters,
                         int numCounters, Component *root,
                         Thread **outThread, unsigned long long *outTimestamp)
{
  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  // TODO: make `GetSubcomponentById` take in an `unsigned int` instead of `int`
  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  if (outThread)
    *outThread = thread;
  rval = StoreCounters<false>(counters, events.get(),
                              std::min(numEvents, numCounters), thread,
                              outTimestamp);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

// TODO: better error handling. Maybe log errors?
std::optional<long long> Thread::GetPAPICounterReading(const std::string &event,
                                                       const std::optional<unsigned long long> &timestamp)
{
  auto metricsIt = attrib.find(metricsKey);
  if (metricsIt == attrib.end())
    return std::nullopt;
  PAPIMetrics *metrics = reinterpret_cast<PAPIMetrics *>( metricsIt->second );

  auto readingsIt = metrics->counters.find(event);
  if (readingsIt == metrics->counters.end())
    return std::nullopt;

  auto &readings = readingsIt->second;
  if (!timestamp)
    return readings.back().second;

  auto it = std::find_if(readings.begin(), readings.end(),
                         [timestamp](const std::pair<unsigned long long, long long> &pair) {
                           return *timestamp == pair.first;
                         }
            );
  if (it == readings.end())
    return std::nullopt;

  return it->second;
}

// TODO: maybe return a pointer to the vector
std::optional<std::vector<std::pair<unsigned long long, long long>>>
Thread::GetAllPAPICounterReadings(const std::string &event)
{
  auto metricsIt = attrib.find(metricsKey);
  if (metricsIt == attrib.end())
    return std::nullopt;
  PAPIMetrics *metrics = reinterpret_cast<PAPIMetrics *>( metricsIt->second );

  auto readingsIt = metrics->counters.find(event);
  if (readingsIt == metrics->counters.end())
    return std::nullopt;

  return readingsIt->second;
}

void Thread::PrintPAPICounters()
{
  auto metricsIt = attrib.find(metricsKey);
  if (metricsIt == attrib.end())
    return;
  PAPIMetrics *metrics = reinterpret_cast<PAPIMetrics *>( metricsIt->second );

  std::cout << "performance counters on thread " << id << ":\n";
  for (const auto &[metric, readings] : metrics->counters)
    std::cout << "  " << metric << ": " << readings.back().second << '\n';
}

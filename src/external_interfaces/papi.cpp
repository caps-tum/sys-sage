#include "sys-sage.hpp"
#include <papi.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <forward_list>
#include <iostream>
#include <memory>
#include <optional>
#include <stddef.h>
#include <sched.h>
#include <string>
#include <sstream>
#include <utility>

#define TIME() ( std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() )

using namespace sys_sage;

static std::optional<unsigned int> GetCpuNumFromTid(unsigned long tid)
{
  static constexpr unsigned hwThreadIdField = 39;

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
  for (unsigned field = 2; field < hwThreadIdField - 1; field++) {
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

  std::string buf (PAPI_MAX_STR_LEN, '\0');

  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf.data());
    if (rval != PAPI_OK)
      return rval;

    std::forward_list<std::pair<unsigned long long, long long>> *readings;
    auto readingsIt = thread->attrib.find(buf);
    if (readingsIt == thread->attrib.end()) {
      readings = new std::forward_list<std::pair<unsigned long long, long long>>;
      thread->attrib[buf] = reinterpret_cast<void *>( readings );
    } else {
      readings = reinterpret_cast<std::forward_list<std::pair<unsigned long long, long long>> *>( readingsIt->second );
    }

    unsigned long long timestamp;
    if constexpr (accumulate) {
      if (readings->empty()) {
        timestamp = TIME();
        readings->emplace_front(timestamp, counters[i]);
      } else {
        auto &pair = readings->front(); // accumulate on the lastest reading
        timestamp = pair.first;
        pair.second += counters[i];
      }
    } else {
      timestamp = TIME();
      readings->emplace_front(timestamp, counters[i]);
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

  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?
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

  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?
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

  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?
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

  Thread *thread = static_cast<Thread *>(
    root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread)
  );
  if (thread == nullptr)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?
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
                                                       unsigned long long timestamp)
{
  auto readingsIt = attrib.find(event);
  if (readingsIt == attrib.end())
    return std::nullopt;

  auto *readings = reinterpret_cast<std::forward_list<std::pair<unsigned long long, long long>> *>( readingsIt->second );
  if (timestamp == 0)
    return readings->front().second;

  auto it = std::find_if(readings->begin(), readings->end(),
                         [timestamp](const std::pair<unsigned long long, long long> &pair) {
                           return timestamp == pair.first;
                         }
            );
  if (it == readings->end())
    return std::nullopt;

  return it->second;
}

std::forward_list<std::pair<unsigned long long, long long>> *
Thread::GetAllPAPICounterReadings(const std::string &event)
{
  auto readingsIt = attrib.find(event);
  if (readingsIt == attrib.end())
    return nullptr;

  return reinterpret_cast<std::forward_list<std::pair<unsigned long long, long long>> *>( readingsIt->second );
}

void Thread::PrintPAPICounters()
{
  int dummy;

  std::cout << "performance counters on thread " << id << ":\n";
  for (const auto &[key, val] : attrib) {
    // indirectly check if key is a valid PAPI event
    if (PAPI_event_name_to_code(key.c_str(), &dummy) != PAPI_OK)
      continue;

    auto *readings = reinterpret_cast<std::forward_list<std::pair<unsigned long long, long long>> *>( val );
    // only print the latest reading
    std::cout << "  " << key << ": " << readings->front().second << '\n';
  }
}

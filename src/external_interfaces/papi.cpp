#include "sys-sage.hpp"
#include <papi.h>
#include <algorithm>
#include <chrono>
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

int sys_sage::PAPI_read(int eventSet, unsigned long long *timestamp,
                        Component *root, PAPIMetrics **metrics)
{
  if (!timestamp || !root || !metrics)
    return PAPI_EINVAL;

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
  if (!thread)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?

  if (!(*metrics))
    *metrics = new PAPIMetrics(thread);
  else if (!(*metrics)->ContainsComponent(thread))
    (*metrics)->AddComponent(thread);

  return (*metrics)->StorePerfCounters<false>(events.get(), numEvents, counters,
                                          timestamp);
}

int sys_sage::PAPI_accum(int eventSet, unsigned long long *timestamp,
                         Component *root, PAPIMetrics **metrics)
{
  if (!timestamp || !root || !metrics)
    return PAPI_EINVAL;

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
  if (!thread)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?

  if (!(*metrics))
    *metrics = new PAPIMetrics(thread);
  else if (!(*metrics)->ContainsComponent(thread))
    (*metrics)->AddComponent(thread);

  return (*metrics)->StorePerfCounters<true>(events.get(), numEvents, counters,
                                          timestamp);
}

int sys_sage::PAPI_stop(int eventSet, unsigned long long *timestamp,
                        Component *root, PAPIMetrics **metrics)
{
  if (!timestamp || !root || !metrics)
    return PAPI_EINVAL;

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
  if (!thread)
    return PAPI_EINVAL; // TODO: is there a better way to handle the error?

  if (!(*metrics))
    *metrics = new PAPIMetrics(thread);
  else if (!(*metrics)->ContainsComponent(thread))
    (*metrics)->AddComponent(thread);

  return (*metrics)->StorePerfCounters<false>(events.get(), numEvents, counters,
                                          timestamp);
}

PAPIMetrics::PAPIMetrics(Thread *thread)
             : Relation ({ thread }, -1, false, RelationType::PAPIMetrics) {}

template <bool accum>
int PAPIMetrics::StorePerfCounters(const int *events, int numEvents,
                                   const long long *counters, 
                                   unsigned long long *timestamp)
{
  int rval;

  unsigned long long ts = TIME();
  char buf[PAPI_MAX_STR_LEN] = { '\0' };

  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf);
    if (rval != PAPI_OK)
      return rval;

    std::vector<std::pair<unsigned long long, long long>> *readings;
    auto readingsIt = attrib.find(buf);

    if (readingsIt == attrib.end()) {
      readings = new std::vector<std::pair<unsigned long long, long long>> { { ts, counters[i] } };
      attrib[buf] = reinterpret_cast<void *>( readings );
      continue;
    }
    readings = reinterpret_cast<std::vector<std::pair<unsigned long long, long long>> *>( readingsIt->second );

    if (*timestamp == 0) {
      readings->emplace_back(ts, counters[i]);
    } else {
      auto &pair = readings->back(); // update the lastest reading
      pair.first = ts;

      if constexpr (accum)
        pair.second += counters[i];
      else
        pair.second = counters[i];
    }
  }

  *timestamp = ts;

  return PAPI_OK;
}

// TODO: maybe use another map instead of `attrib` to use integer keys?
long long PAPIMetrics::GetPerfCounterReading(int event,
                                             unsigned long long timestamp)
{
  int rval;

  char buf[PAPI_MAX_STR_LEN];
  rval = PAPI_event_code_to_name(event, buf);
  if (rval != PAPI_OK)
    return -1;

  auto readingsIt = attrib.find(buf);
  if (readingsIt == attrib.end())
    return -1;

  auto *readings = reinterpret_cast<std::vector<std::pair<unsigned long long, long long>> *>( readingsIt->second );
  if (timestamp == 0)
    return readings->back().second;

  auto it = std::find_if(readings->rbegin(), readings->rend(),
                         [timestamp](const std::pair<unsigned long long, long long> &pair) {
                           return timestamp == pair.first;
                         }
            );
  if (it == readings->rend())
    return -1;

  return it->second;
}

// TODO: maybe use another map instead of `attrib` to use integer keys?
std::vector<std::pair<unsigned long long, long long>> *
PAPIMetrics::GetPerfCounterReadings(int event)
{
  int rval;

  char buf[PAPI_MAX_STR_LEN];
  rval = PAPI_event_code_to_name(event, buf);
  if (rval != PAPI_OK)
    return nullptr;

  auto readingsIt = attrib.find(buf);
  if (readingsIt == attrib.end())
    return nullptr;

  return reinterpret_cast<std::vector<std::pair<unsigned long long, long long>> *>( readingsIt->second );
}

void PAPIMetrics::PrintLatestPerfCounterReadings()
{
  std::cout << "performance counters monitored on HW thread(s):";
  for (Component *c : components)
    std::cout << ' ' << static_cast<Thread *>(c)->GetId();
  std::cout << '\n';

  for (const auto &[key, val] : attrib) {
    auto *readings = reinterpret_cast<std::vector<std::pair<unsigned long long, long long>> *>( val );
    std::cout << "  " << key << ": " << readings->back().second << '\n';
  }
}

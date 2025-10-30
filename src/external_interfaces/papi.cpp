// idea: enable the user to make an entry permanent or temporary

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
#include <vector>

#define TIME() ( std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() )

using namespace sys_sage;

struct CpuPerf {
  std::vector<PerfEntry> entries;
  int cpuNum;
};

static std::optional<int> GetCpuNumFromTid(unsigned long tid)
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
  int cpuNum;
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

int sys_sage::SS_PAPI_start(int eventSet, PAPIMetrics **metrics)
{
  if (!metrics)
    return PAPI_EINVAL;

  int rval;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK)
    return rval;

  if (!(*metrics))
    *metrics = new PAPIMetrics;
  else
    (*metrics)->reset = true; // PAPI_start will reset the counters

  return PAPI_OK;
}

int sys_sage::SS_PAPI_reset(int eventSet, PAPIMetrics *metrics)
{
  if (!metrics)
    return PAPI_EINVAL;

  int rval;

  rval = PAPI_reset(eventSet);
  if (rval != PAPI_OK)
    return rval;

  metrics->reset = true;

  return PAPI_OK;
}

int sys_sage::SS_PAPI_read(int eventSet, unsigned long long *timestamp,
                           Component *root, PAPIMetrics *metrics)
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
  rval = PAPI_read(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  Thread *thread = root->GetSubcomponentById(cpuNum, ComponentType::Thread);
  if (!thread)
    return PAPI_EINVAL; // TODO: better error handling

  return StorePerfCounters<false>(metrics, events.get(), numEvents, counters,
                                  timestamp, thread);
}

int sys_sage::SS_PAPI_accum(int eventSet, unsigned long long *timestamp,
                            Component *root, PAPIMetrics *metrics)
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
  rval = PAPI_accum(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  Thread *thread = root->GetSubcomponentById(cpuNum, ComponentType::Thread);
  if (!thread)
    return PAPI_EINVAL; // TODO: better error handling

  return AccumPerfCounters(metrics, events.get(), numEvents, counters,
                           timestamp, thread);
}

int sys_sage::SS_PAPI_stop(int eventSet, unsigned long long *timestamp,
                           Component *root, PAPIMetrics *metrics)
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
  rval = PAPI_stop(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  Thread *thread = root->GetSubcomponentById(cpuNum, ComponentType::Thread);
  if (!thread)
    return PAPI_EINVAL; // TODO: better error handling

  return StorePerfCounters(metrics, events.get(), numEvents, counters,
                           timestamp, thread);
}

PAPIMetrics::PAPIMetrics() : Relation (RelationType::PAPIMetrics) {}

static void DeletePerfEntries(PAPIMetrics *metrics)
{
  auto eventIt = metrics->attrib.begin();
  while (eventIt != metrics->attrib.end()) { // iterate over events
    auto *cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( *eventIt );

    auto cpuPerfIt = cpuPerfs->begin();
    while (cpuPerfIt != cpuPerfs->end()) { // iterate over the CPUs
      // only the last entry is relevant, since the prior once are always
      // permanent
      PerfEntry &lastEntry = cpuPerfIt->entries.back();
      if (!lastEntry.permanent) {
        cpuPerfIt->entries.pop_back();
        if (cpuPerfIt->entries.size() == 0) {
          // TODO: introduce a reference counter for the CPUs of the relation
          //       -> if (cpuPerf.entries.size() == 0) { cpu_ref_counter--; }
          //       -> if (cpu_ref_counter == 0) { <erase_cpu_from_component_list>; }
          cpuPerfIt = cpuPerfs->erase(cpuPerfIt);
          continue;
        }
      }
      cpuPerfIt++;
    }

    if (cpuPerfs->size() == 0) // no entries left for the event
      eventIt = metrics->attrib.erase(eventIt);
    else
      eventIt++;
  }
}

static int StorePerfCounters(PAPIMetrics *metrics, const int *events,
                             int numEvents, const long long *counters, 
                             unsigned long long *timestamp, Thread *thread)
{
  int rval;

  if (metrics->reset) {
    DeletePerfEntries(metrics);
    metrics->reset = false;
  }

  if (!metrics->ContainsComponent(thread))
    metrics->AddComponent(thread); // TODO: add reference counting

  bool permanent = *timestamp == 0;
  *timestamp = TIME();

  char buf[PAPI_MAX_STR_LEN] = { '\0' };
  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf);
    if (rval != PAPI_OK)
      return rval;

    std::vector<CpuPerf> *cpuPerfs;
    auto cpuPerfsIt = metrics->attrib.find(buf);

    if (cpuPerfsIt == metrics->attrib.end()) { // no entries for this event
      PerfEntry perfEntry { .timestamp = *timestamp, .value = counters[i], .permanent = permanent };
      cpuPerfs = new std::vector<CpuPerf> {
        // create a single CpuPerf which contains a single PerfEntry
        {
          .entries = { perfEntry },
          .cpuNum = thread->GetId()
        }
      };

      metrics->attrib[buf] = reinterpret_cast<void *>( cpuPerfs );
      continue;
    }
    cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( cpuPerfsIt->second );


    long long sum = 0;

    int idx = -1;
    for (int = 0; i < cpuPerfs->size(); i++) {
      CpuPerf &cpuPerf = (*cpuPerfs)[i];
      PerfEntry &lastEntry = cpuPerf.entries.back();

      if (cpuPerf.cpuNum == thread->GetId()) {
        idx = i;
        if (!lastEntry.permanent)
          continue;
      }

      // if it's permanent, we can't modify it
      // -> simply include the value
      if (!lastEntry.permanent)
        sum += lastEntry.value;
    }

    long long value = counters[i] - sum;

    if (idx < 0) {
      PerfEntry perfEntry { .timestamp = *timestamp, .value = value, .permanent = permanent };
      cpuPerfs.entries.emplace_back(
        {
          .entries = { perfEntry },
          .cpuNum = thread->GetId()
        }
      );
    } else {
      CpuPerf &cpuPerf = (*cpuPerfs)[idx];
      PerfEntry &lastEntry = cpuPerf.entries.back();

      if (lastEntry.permanent) {
        PerfEntry perfEntry { .timestamp = *timestamp, .value = value, .permanent = permanent };
        cpuPerfs.entries.emplace_back(
          {
            .entries = { perfEntry },
            .cpuNum = thread->GetId()
          }
        );
      } else {
        lastEntry.timestamp = *timestamp;
        lastEntry.value = value;
        lastEntry.permanent = permanent;
      }
    }
  }

  metrics->latestTimestamp = *timestamp;

  return PAPI_OK;
}

static int AccumPerfCounters(PAPIMetrics *metrics, const int *events,
                             int numEvents, const long long *counters, 
                             unsigned long long *timestamp, Thread *thread)
{
  int rval;

  metrics->reset = true;

  if (!metrics->ContainsComponent(thread))
    metrics->AddComponent(thread); // TODO: add reference counting

  bool permanent = *timestamp == 0;
  *timestamp = TIME();

  char buf[PAPI_MAX_STR_LEN] = { '\0' };
  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf);
    if (rval != PAPI_OK)
      return rval;

    std::vector<CpuPerf> *cpuPerfs;
    auto cpuPerfsIt = metrics->attrib.find(buf);

    if (cpuPerfsIt == metrics->attrib.end()) { // no entries for this event
      PerfEntry perfEntry { .timestamp = ts, .value = counters[i], .permanent = permanent };
      cpuPerfs = new std::vector<CpuPerf> {
        // create a single CpuPerf which contains a single PerfEntry
        {
          .entries = { perfEntry },
          .cpuNum = thread->GetId()
        }
      };

      metrics->attrib[buf] = reinterpret_cast<void *>( cpuPerfs );
      continue;
    }
    cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( cpuPerfsIt->second );

    auto cpuPerfIt = std::find_if(
                       cpuPerfs->begin(), cpuPerfs->end(),
                       [thread](const CpuPerf &cpuPerf) {
                         return cpuPerf.cpuNum == thread->GetId();
                       }
                     );
    if (cpuPerfIt == cpuPerfs->end()) {
      PerfEntry perfEntry { .timestamp = *timestamp, .value = counters[i], .permanent = permanent };
      cpuPerfs->entries.emplace_back(
        {
          .entries = { perfEntry },
          .cpuNum = thread->GetId()
        }
      );
    } else {
      CpuPerf &cpuPerf = cpuPerfIt->second;
      PerfEntry lastEntry = cpuPerf.entries.back();

      if (lastEntry.permanent) {
        PerfEntry perfEntry { .timestamp = *timestamp, .value = lastEntry.value + counters[i], .permanent = permanent };
        cpuPerf.entries.emplace_back(perfEntry);
      } else {
        lastEntry.timestamp = *timestamp;
        lastEntry.value += counters[i];
        lastEntry.permanent = permanent;
      }
    }
  }

  metrics->latestTimestamp = *timestamp;

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

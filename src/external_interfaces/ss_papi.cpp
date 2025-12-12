#include "sys-sage.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <stddef.h>
#include <sched.h>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#define TIME() ( std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() )

using namespace sys_sage;

struct MetaData {
  std::unordered_map<int, int> cpuReferenceCounters;
  unsigned long long latestTimestamp = 0;
  int eventSet;
  bool reset = true;
};

static const char *metaKey = "meta";

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

// hope the compiler is smart enough to inline this and to create the objects in-place
static inline void AppendNewCpuPerf(std::vector<CpuPerf> *cpuPerfs,
                                    std::unordered_map<int, int> &refCounters,
                                    unsigned long long timestamp,
                                    long long value, bool permanent, int cpuNum)
{
  PerfEntry perfEntry {
    .timestamp = timestamp,
    .value = value,
    .permanent = permanent
  };

  cpuPerfs->push_back(
    {
      .perfEntries = std::vector<PerfEntry> { std::move(perfEntry) },
      .cpuNum = cpuNum
    }
  );

  refCounters[cpuNum]++;
}

static inline void RemoveCpu(Relation *metrics, int cpuNum)
{
  const std::vector<Component *> &components = metrics->GetComponents();
  
  auto cpuIt = std::find_if(components.begin(), components.end(),
                            [cpuNum](const Component *component)
                            {
                              return component->GetId() == cpuNum;
                            }
               );

  metrics->RemoveComponent(cpuIt - components.begin());
}

static void DeletePerfEntries(Relation *metrics)
{
  auto meta = reinterpret_cast<MetaData *>( metrics->attrib[metaKey] );

  int code;

  auto it = metrics->attrib.begin();
  while (it != metrics->attrib.end()) {
    if (PAPI_event_name_to_code(it->first.c_str(), &code) != PAPI_OK) { // check if attribute is a PAPI event
      it++;
      continue;
    }

    auto *cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( it->second );

    auto cpuPerfIt = cpuPerfs->begin();
    while (cpuPerfIt != cpuPerfs->end()) { // iterate over the CPUs
      PerfEntry &lastEntry = cpuPerfIt->perfEntries.back();

      if (!lastEntry.permanent) {
        cpuPerfIt->perfEntries.pop_back();
        if (cpuPerfIt->perfEntries.size() == 0) {
          auto refCountIt = meta->cpuReferenceCounters.find(cpuPerfIt->cpuNum);
          if (refCountIt->second == 1) {
            meta->cpuReferenceCounters.erase(refCountIt);
            RemoveCpu(metrics, cpuPerfIt->cpuNum);
          } else {
            refCountIt->second--;
          }

          cpuPerfIt = cpuPerfs->erase(cpuPerfIt);
          continue;
        }
      }
      cpuPerfIt++;
    }

    if (cpuPerfs->size() == 0) { // no entries left for the event
      delete cpuPerfs;
      it = metrics->attrib.erase(it);
    } else {
      it++;
    }
  }
}

static int StorePerfCounters(Relation *metrics, const int *events, int numEvents,
                             const long long *counters, Thread *cpu,
                             bool permanent, unsigned long long *timestamp)
{
  auto meta = reinterpret_cast<MetaData *>( metrics->attrib.find(metaKey)->second );

  int rval;

  if (meta->reset) {
    DeletePerfEntries(metrics);
    meta->reset = false;
  }

  if (!metrics->ContainsComponent(cpu)) {
    metrics->AddComponent(cpu);
    meta->cpuReferenceCounters[cpu->GetId()] = 0;
  }

  unsigned long long ts = TIME();
  char buf[PAPI_MAX_STR_LEN] = { '\0' };

  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf);
    if (rval != PAPI_OK)
      return rval;

    std::vector<CpuPerf> *cpuPerfs;

    auto eventIt = metrics->attrib.find(buf);
    if (eventIt == metrics->attrib.end()) {
      cpuPerfs = new std::vector<CpuPerf>;
      AppendNewCpuPerf(cpuPerfs, meta->cpuReferenceCounters, ts, counters[i], permanent, cpu->GetId());
      metrics->attrib[buf] = reinterpret_cast<void *>( cpuPerfs );

      continue;
    }
    
    cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( eventIt->second );

    long long sum = 0;
    auto cpuPerfIt = cpuPerfs->end();
    for (auto it = cpuPerfs->begin(); it != cpuPerfs->end(); it++) {
      if (it->cpuNum == cpu->GetId()) {
        cpuPerfIt = it;
        continue;
      }

      PerfEntry &lastEntry = it->perfEntries.back();

      if (lastEntry.timestamp == meta->latestTimestamp && !lastEntry.permanent) {
        sum += lastEntry.value;
        lastEntry.timestamp = ts;
      }
    }

    long long value = counters[i] - sum;

    if (cpuPerfIt == cpuPerfs->end()) {
      AppendNewCpuPerf(cpuPerfs, meta->cpuReferenceCounters, ts, value, permanent, cpu->GetId());
    } else {
      PerfEntry &lastEntry = cpuPerfIt->perfEntries.back();

      if (lastEntry.permanent) {
        cpuPerfIt->perfEntries.emplace_back(ts, value, permanent);
      } else {
        lastEntry.timestamp = ts;
        lastEntry.value = value;
        lastEntry.permanent = permanent;
      }
    }
  }

  meta->latestTimestamp = ts;

  if (timestamp)
    *timestamp = ts;

  return PAPI_OK;
}

static int AccumPerfCounters(Relation *metrics, const int *events, int numEvents,
                             const long long *counters,  Thread *cpu,
                             bool permanent, unsigned long long *timestamp)
{
  auto meta = reinterpret_cast<MetaData *>( metrics->attrib.find(metaKey)->second );

  int rval;

  meta->reset = true;

  if (!metrics->ContainsComponent(cpu)) {
    metrics->AddComponent(cpu);
    meta->cpuReferenceCounters[cpu->GetId()] = 0;
  }

  unsigned long long ts = TIME();
  char buf[PAPI_MAX_STR_LEN] = { '\0' };

  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf);
    if (rval != PAPI_OK)
      return rval;

    std::vector<CpuPerf> *cpuPerfs;

    auto eventIt = metrics->attrib.find(buf);
    if (eventIt == metrics->attrib.end()) {
      cpuPerfs = new std::vector<CpuPerf>;
      AppendNewCpuPerf(cpuPerfs, meta->cpuReferenceCounters, ts, counters[i], permanent, cpu->GetId());
      metrics->attrib[buf] = reinterpret_cast<void *>( cpuPerfs );

      continue;
    }
    
    cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( eventIt->second );

    long long sum = 0;
    auto cpuPerfIt = cpuPerfs->end();
    for (auto it = cpuPerfs->begin(); it != cpuPerfs->end(); it++) {
      if (it->cpuNum == cpu->GetId()) {
        cpuPerfIt = it;
        continue;
      }

      PerfEntry &lastEntry = it->perfEntries.back();

      if (lastEntry.timestamp == meta->latestTimestamp) {
        if (lastEntry.permanent)
          sum += lastEntry.value;
        else
          lastEntry.timestamp = ts;
      }
    }

    long long value = counters[i] + sum;

    if (cpuPerfIt == cpuPerfs->end()) {
      AppendNewCpuPerf(cpuPerfs, meta->cpuReferenceCounters, ts, value, permanent, cpu->GetId());
    } else {
      PerfEntry &lastEntry = cpuPerfIt->perfEntries.back();

      if (lastEntry.permanent) {
        if (lastEntry.timestamp == meta->latestTimestamp)
          value += lastEntry.value;
        cpuPerfIt->perfEntries.emplace_back(ts, value, permanent);
      } else {
        lastEntry.timestamp = ts;
        lastEntry.value += value;
        lastEntry.permanent = permanent;
      }
    }
  }

  meta->latestTimestamp = ts;

  if (timestamp)
    *timestamp = ts;

  return PAPI_OK;
}

int sys_sage::SS_PAPI_start(int eventSet, Relation **metrics)
{
  if (!metrics)
    return PAPI_EINVAL;

  int rval;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK)
    return rval;

  if (!(*metrics)) {
    std::vector<Component *> empty {};
    *metrics = new Relation(empty, 0, false, RelationCategory::PAPI_Metrics);

    (*metrics)->attrib[metaKey] = reinterpret_cast<void *>( new MetaData{ .eventSet = eventSet } );
  } else {
    if ((*metrics)->GetCategory() != RelationCategory::PAPI_Metrics)
      return PAPI_EINVAL;

    auto meta = reinterpret_cast<MetaData *>((*metrics)->attrib[metaKey]);
    meta->eventSet = eventSet;
    meta->reset = true; // PAPI_start will reset the counters
  }

  return PAPI_OK;
}

int sys_sage::SS_PAPI_reset(Relation *metrics)
{
  if (!metrics || metrics->GetCategory() != RelationCategory::PAPI_Metrics)
    return PAPI_EINVAL;

  auto meta = reinterpret_cast<MetaData *>( metrics->attrib[metaKey] );

  int rval;

  rval = PAPI_reset(meta->eventSet);
  if (rval != PAPI_OK)
    return rval;

  meta->reset = true;

  return PAPI_OK;
}

int sys_sage::SS_PAPI_read(Relation *metrics, Component *root, bool permanent,
                           unsigned long long *timestamp)
{
  if (!metrics || metrics->GetCategory() != RelationCategory::PAPI_Metrics || !root)
    return PAPI_EINVAL;

  auto meta = reinterpret_cast<MetaData *>( metrics->attrib[metaKey] );

  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(meta->eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  long long counters[numEvents];
  rval = PAPI_read(meta->eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(meta->eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  Thread *cpu = static_cast<Thread *>( root->GetSubcomponentById(cpuNum, ComponentType::Thread) );
  if (!cpu)
    return PAPI_EINVAL; // TODO: better error handling

  return StorePerfCounters(metrics, events.get(), numEvents, counters, cpu, permanent, timestamp);
}

int sys_sage::SS_PAPI_accum(Relation *metrics, Component *root, bool permanent,
                            unsigned long long *timestamp)
{
  if (!metrics || metrics->GetCategory() != RelationCategory::PAPI_Metrics || !root)
    return PAPI_EINVAL;

  auto meta = reinterpret_cast<MetaData *>( metrics->attrib[metaKey] );

  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(meta->eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  long long counters[numEvents] = { 0 };
  rval = PAPI_accum(meta->eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(meta->eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  Thread *cpu = static_cast<Thread *>( root->GetSubcomponentById(cpuNum, ComponentType::Thread) );
  if (!cpu)
    return PAPI_EINVAL; // TODO: better error handling

  return AccumPerfCounters(metrics, events.get(), numEvents, counters, cpu, permanent, timestamp);
}

int sys_sage::SS_PAPI_stop(Relation *metrics, Component *root, bool permanent,
                           unsigned long long *timestamp)
{
  if (!metrics || metrics->GetCategory() != RelationCategory::PAPI_Metrics || !root)
    return PAPI_EINVAL;

  auto meta = reinterpret_cast<MetaData *>( metrics->attrib[metaKey] );

  int rval;

  std::unique_ptr<int[]> events;
  int numEvents;
  rval = GetEvents(meta->eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  long long counters[numEvents];
  rval = PAPI_stop(meta->eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(meta->eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  Thread *cpu = static_cast<Thread *>( root->GetSubcomponentById(cpuNum, ComponentType::Thread) );
  if (!cpu)
    return PAPI_EINVAL; // TODO: better error handling

  return StorePerfCounters(metrics, events.get(), numEvents, counters, cpu, permanent, timestamp);
}

long long sys_sage::Relation::GetPAPImetric(int event, int cpuNum,
                                            unsigned long long timestamp) const
{
  if (category != RelationCategory::PAPI_Metrics)
    return 0;

  auto meta = reinterpret_cast<MetaData *>( attrib.find(metaKey)->second );

  int rval;

  char buf[PAPI_MAX_STR_LEN];
  rval = PAPI_event_code_to_name(event, buf);
  if (rval != PAPI_OK)
    return 0;

  auto eventIt = attrib.find(buf);
  if (eventIt == attrib.end())
    return 0;

  auto *cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( eventIt->second );
  unsigned long long targetTimestamp = timestamp == 0 ? meta->latestTimestamp : timestamp;
  long long value = 0;

  for (auto it = cpuPerfs->begin(); it != cpuPerfs->end(); it++) {
    if (cpuNum < 0 || it->cpuNum == cpuNum) {
      auto perfEntryIt = std::find_if(it->perfEntries.rbegin(), it->perfEntries.rend(),
                                      [targetTimestamp](const PerfEntry &perfEntry)
                                      {
                                        return perfEntry.timestamp == targetTimestamp;
                                      }
                         );
      if (perfEntryIt == it->perfEntries.rend())
        continue;

      value += perfEntryIt->value;

      if (it->cpuNum == cpuNum)
        break;
    }
  }

  return value;
}

const CpuPerf *sys_sage::Relation::GetAllPAPImetrics(int event, int cpuNum) const
{
  if (category != RelationCategory::PAPI_Metrics)
    return 0;

  int rval;

  char buf[PAPI_MAX_STR_LEN];
  rval = PAPI_event_code_to_name(event, buf);
  if (rval != PAPI_OK)
    return nullptr;

  auto eventIt = attrib.find(buf);
  if (eventIt == attrib.end())
    return nullptr;

  auto *cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( eventIt->second );
  auto cpuPerfIt = std::find_if(cpuPerfs->begin(), cpuPerfs->end(),
                                [cpuNum](const CpuPerf &cpuPerf)
                                {
                                  return cpuPerf.cpuNum == cpuNum;
                                }
                   );

  if (cpuPerfIt == cpuPerfs->end())
    return nullptr;

  return &(*cpuPerfIt);
}

void sys_sage::Relation::PrintAllPAPImetrics() const
{
  if (category != RelationCategory::PAPI_Metrics)
    return;
  
  int code;

  for (auto cpu : components) {
    int cpuNum = cpu->GetId();
    std::cout << "metrics on CPU " << cpuNum << ":\n";

    for (auto &[key, val] : attrib) {
      if (PAPI_event_name_to_code(key.c_str(), &code) != PAPI_OK) // check if attribute is a PAPI event
        continue;

      std::cout << "  " << key << ":\n";
      
      auto cpuPerfs = reinterpret_cast<std::vector<CpuPerf> *>( val );
      auto cpuPerfIt = std::find_if(cpuPerfs->begin(), cpuPerfs->end(),
                                    [cpuNum](const CpuPerf &cpuPerf)
                                    {
                                      return cpuPerf.cpuNum == cpuNum;
                                    }
                       );

      for (auto &perfEntry : cpuPerfIt->perfEntries)
        std::cout << "    " << perfEntry << '\n';
    }
  }
}

std::ostream &operator<<(std::ostream &stream, const PerfEntry &perfEntry)
{
  return stream << "{ .timestamp = " << perfEntry.timestamp << ", .value = " << perfEntry.value << " }";
}

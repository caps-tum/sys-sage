#include "sys-sage.hpp"
#include <papi.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <stddef.h>
#include <sched.h>
#include <string>
#include <sstream>
#include <sys/syscall.h>
#include <utility>
#include <unistd.h>

using namespace sys_sage;

static const std::string metricsKey ( "PAPI_Metrics" );

struct PAPIMetrics {
  std::unordered_map<std::string, long long> values;
};

template <typename T>
struct RaiiArray {
  RaiiArray()
  {
    array = nullptr;
  }
  
  RaiiArray(size_t size)
  {
    array = new T[size];
  }
  
  ~RaiiArray()
  {
    delete[] array;
  }
  
  RaiiArray &operator=(RaiiArray &&movingObject) noexcept
  {
    if (this != &movingObject) {
      delete[] array;
      array = movingObject.array;
      movingObject.array = nullptr;
    }
    return *this;
  }

  T *array;
};

static std::optional<unsigned int> GetCpuNumFromTid(unsigned long tid)
{
  static constexpr int hwThreadIdField = 39;

  long lrval = syscall(SYS_gettid);
  if (lrval < 0)
    return std::nullopt;

  if (static_cast<unsigned long>(lrval) == tid) {
    int rval = sched_getcpu();
    if (rval < 0)
      return std::nullopt;
    return rval;
  }

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

  opt.attach.eventset = eventSet;
  opt.attach.tid = PAPI_NULL;
  rval = PAPI_get_opt(PAPI_ATTACH, &opt);
  if (rval < 0) {
    return rval;
  } else if (static_cast<bool>(rval) == true) {
    if ( std::optional<unsigned int> optCpuNum = GetCpuNumFromTid(opt.attach.tid) ) {
      *cpuNum = *optCpuNum;
      return PAPI_OK;
    }
    return PAPI_EINVAL;
  }

  opt.cpu.eventset = eventSet;
  opt.cpu.cpu_num = PAPI_NULL;
  rval = PAPI_get_opt(PAPI_CPU_ATTACH, &opt);
  if (rval < 0) {
    return rval;
  } else if (static_cast<bool>(rval) == true) {
    *cpuNum = opt.cpu.cpu_num;
    return PAPI_OK;
  }

  return PAPI_ECOMBO;
}

static int GetEvents(int eventSet, RaiiArray<int> &events, int *numEvents)
{
  int rval;

  rval = PAPI_num_events(eventSet);
  if (rval < 0)
    return rval;
  else if (rval == 0)
    return PAPI_EINVAL;
  int tmpNumEvents = rval;

  RaiiArray<int> tmpEvents (tmpNumEvents);
  rval = PAPI_list_events(eventSet, tmpEvents.array, &tmpNumEvents);
  if (rval != PAPI_OK)
    return rval;

  events = std::move(tmpEvents);
  *numEvents = tmpNumEvents;
  return PAPI_OK;
}

template <bool accumulate>
static int StoreCounters(const long long *counters, const int *events,
                         int numEvents, Thread *thread)
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

    auto it = metrics->values.find(buf);
    if (it == metrics->values.end()) {
      metrics->values[buf] = counters[i];
    } else {
      if constexpr (accumulate)
        it->second += counters[i];
      else
        it->second = counters[i];
    }
  }

  return PAPI_OK;
}

int sys_sage::PAPI_read(int eventSet, Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
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
  Thread *thread = static_cast<Thread *>( root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread) );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  *outThread = thread;
  rval = StoreCounters<false>(counters, events.array, numEvents, thread);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

int sys_sage::PAPI_accum(int eventSet, Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
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
  Thread *thread = static_cast<Thread *>( root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread) );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  *outThread = thread;
  rval = StoreCounters<true>(counters, events.array, numEvents, thread);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

int sys_sage::PAPI_stop(int eventSet, Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
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
  Thread *thread = static_cast<Thread *>( root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread) );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  *outThread = thread;
  rval = StoreCounters<false>(counters, events.array, numEvents, thread);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

int sys_sage::PAPI_store(int eventSet, const long long *counters, int numCounters,
                         Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
  int numEvents;
  rval = GetEvents(eventSet, events, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  unsigned int cpuNum;
  rval = GetCpuNum(eventSet, &cpuNum);
  if (rval != PAPI_OK)
    return rval;

  // TODO: make `GetSubcomponentById` take in an `unsigned int` instead of `int`
  Thread *thread = static_cast<Thread *>( root->GetSubcomponentById(static_cast<int>(cpuNum), ComponentType::Thread) );
  if (thread == nullptr)
    // TODO: is there a better way to handle the error?
    return PAPI_EINVAL;
  *outThread = thread;
  rval = StoreCounters<false>(counters, events.array, std::min(numEvents, numCounters), thread);
  if (rval != PAPI_OK)
    return rval;

  return PAPI_OK;
}

// TODO: better error handling. Maybe log errors?
std::optional<long long> Thread::GetPAPICounter(const std::string &event)
{
  auto metricsIt = attrib.find(metricsKey);
  if (metricsIt == attrib.end())
    return std::nullopt;
  PAPIMetrics *metrics = reinterpret_cast<PAPIMetrics *>( metricsIt->second );

  auto it = metrics->values.find(event);
  if (it == metrics->values.end())
    return std::nullopt;

  return it->second;
}

void Thread::PrintPAPICounters()
{
  auto metricsIt = attrib.find(metricsKey);
  if (metricsIt == attrib.end())
    return;
  PAPIMetrics *metrics = reinterpret_cast<PAPIMetrics *>( metricsIt->second );

  for (const auto &[metric, value] : metrics->values)
    std::cout << metric << ": " << value << '\n';
}

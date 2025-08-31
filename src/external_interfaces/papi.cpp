#include "sys-sage.hpp"
#include <papi.h>
#include <stddef.h>
#include <string>
#include <sched.h>
#include <utility>

using namespace sys_sage;

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
  
  T &operator[](size_t index)
  {
    return array[index];
  }
  
  const T &operator[](size_t index) const
  {
    return array[index];
  }
  
  RaiiArray &operator=(RaiiArray &&movingObject) noexcept
  {
    if (this != &movingObject)
      std::swap(array, movingObject.array);
    return *this;
  }

  T *array;
};

static constexpr int FieldTaskCpu = 39;
static constexpr int MaxStatFileSize = 1024;

static int getField(const std::string& fields, int field) {
  int value = -1;
  auto pos = fields.find_last_of(')');
  if ( pos != std::string::npos ) {
    int i;
    for(i=2, pos = pos + 1; 
      i<field-1 && pos != std::string::npos; 
      i++, pos = fields.find_first_of(' ', pos + 1) ) {}
    if ( i == field-1 && pos != std::string::npos ) {
      pos++;
      const auto pos2 = fields.find_first_of(' ', pos);
      if (pos2 != std::string::npos) {
        auto field = fields.substr(pos, pos2-pos);
        std::stringstream sstream{field};
        sstream >> value;
      }
    }
  }
  return value;
}

int GetCpuNumFromTid(int tid) {
  if ( tid == 0 ) return sched_getcpu();
  
  int cpu = -1;
  std::string path = "/proc/" + std::to_string(tid) + "/stat";
  std::ifstream statFile{path, std::ios::in | std::ios::binary};
  if ( statFile.is_open() ) {
    std::string stat(MaxStatFileSize, '\0');
    statFile.read(stat.data(), MaxStatFileSize);
    stat.resize(statFile.gcount());
    cpu = getField(stat, FieldTaskCpu);
    statFile.close();
  }
  return cpu;
}

static inline int GetCpuNum(int eventSet)
{
  PAPI_option_t opt;
  opt.attach.eventset = eventSet;
  opt.attach.tid = PAPI_NULL;
  opt.cpu.eventset = eventSet;
  opt.cpu.cpu_num = PAPI_NULL;

  if (PAPI_get_opt(PAPI_ATTACH, &opt) == PAPI_OK)
    return GetCpuNumFromTid(opt.attach.tid);
  else if (PAPI_get_opt(PAPI_CPU_ATTACH, &opt) == PAPI_OK)
    return opt.cpu.cpu_num;
  else
    return PAPI_EINVAL;
}

static inline int GetEvents(int eventSet, RaiiArray<int> &events)
{
  int rval;

  rval = PAPI_num_events(eventSet);
  if (rval < PAPI_OK)
    return rval;
  if (rval == 0)
    return PAPI_EINVAL;

  int numEvents = rval;
  RaiiArray<int> tmpEvents (numEvents);
  rval = PAPI_list_events(eventSet, tmpEvents.array, &numEvents);
  if (rval != PAPI_OK)
    return rval;

  events = std::move(tmpEvents);
  return numEvents;
}

template <bool accumulate>
int StoreCounters(const long long *counters, const int *events, int numEvents,
                  Thread *thread)
{
  int rval;
  char buf[PAPI_MAX_STR_LEN] = { 0 };

  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], buf);
    if (rval != PAPI_OK)
      return rval;

    auto it = thread->attrib.find(buf);
    if (it == thread->attrib.end()) {
      thread->attrib[buf] = reinterpret_cast<void *>( new long long (counters[i]) );
    } else {
      if constexpr (accumulate)
        *reinterpret_cast<long long *>(it->second) += counters[i];
      else
        *reinterpret_cast<long long *>(it->second) = counters[i];
    }
  }

  // TODO: settle for unified error codes in the sys-sage PAPI integration
  return PAPI_OK;
}

int SYSSAGE_PAPI_read(int eventSet, Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
  rval = GetEvents(eventSet, events);
  if (rval <= 0)
    return rval;
  int numEvents = rval;

  long long counters[numEvents];
  rval = PAPI_read(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  rval = GetCpuNum(eventSet);
  if (rval < PAPI_OK)
    return rval;
  int cpuNum = rval;

  Thread *thread = reinterpret_cast<Thread *>(root->GetSubcomponentById(cpuNum, ComponentType::Thread));
  // TODO: settle for unified error codes in the sys-sage PAPI integration
  if (thread == nullptr)
    return -1;

  rval = StoreCounters<false>(counters, events.array, numEvents, thread);
  if (rval != PAPI_OK)
    return rval;

  *outThread = thread;

  return PAPI_OK;
}

int SYSSAGE_PAPI_accum(int eventSet, Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
  rval = GetEvents(eventSet, events);
  if (rval <= 0)
    return rval;
  int numEvents = rval;

  long long counters[numEvents] = {0};
  rval = PAPI_accum(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  rval = GetCpuNum(eventSet);
  if (rval < PAPI_OK)
    return rval;
  int cpuNum = rval;

  Thread *thread = reinterpret_cast<Thread *>(root->GetSubcomponentById(cpuNum, ComponentType::Thread));
  // TODO: settle for unified error codes in the sys-sage PAPI integration
  if (thread == nullptr)
    return -1;

  rval = StoreCounters<true>(counters, events.array, numEvents, thread);
  if (rval != PAPI_OK)
    return rval;

  *outThread = thread;

  return PAPI_OK;
}

int SYSSAGE_PAPI_stop(int eventSet, Component *root, Thread **outThread)
{
  int rval;

  RaiiArray<int> events;
  rval = GetEvents(eventSet, events);
  if (rval <= 0)
    return rval;
  int numEvents = rval;

  long long counters[numEvents];
  rval = PAPI_stop(eventSet, counters);
  if (rval != PAPI_OK)
    return rval;

  rval = GetCpuNum(eventSet);
  if (rval < PAPI_OK)
    return rval;
  int cpuNum = rval;

  Thread *thread = reinterpret_cast<Thread *>(root->GetSubcomponentById(cpuNum, ComponentType::Thread));
  // TODO: settle for unified error codes in the sys-sage PAPI integration
  if (thread == nullptr)
    return -1;

  rval = StoreCounters<false>(counters, events.array, numEvents, thread);
  if (rval != PAPI_OK)
    return rval;

  *outThread = thread;

  return PAPI_OK;
}

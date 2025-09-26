#include "sys-sage.hpp"
#include <papi.h>
#include <assert.h>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>

#define FATAL(errMsg) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  return EXIT_FAILURE;\
} while (false)

static constexpr int hwThreadId = 3;

struct worker_args {
  sys_sage::Component *topoRoot;
  sys_sage::Thread *outThread;
  int eventSet;
  int rval;
};

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
  for (size_t i = 0; i < n; i++)
    a[i] = alpha * b[i] + c[i];
}

void *work(void *arg)
{
  worker_args *wargs = reinterpret_cast<worker_args *>( arg );

  wargs->rval = PAPI_register_thread();
  if (wargs->rval != PAPI_OK)
    return nullptr;

  size_t n = 1'000'000;
  auto a = std::make_unique<double[]>(n);
  auto b = std::make_unique<double[]>(n);
  auto c = std::make_unique<double[]>(n);
  double alpha = 3.14159;

  wargs->rval = PAPI_start(wargs->eventSet);
  if (wargs->rval != PAPI_OK)
    return nullptr;

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  wargs->rval = sys_sage::PAPI_stop(wargs->eventSet, wargs->topoRoot,
                                    &wargs->outThread);
  if (wargs->rval != PAPI_OK)
    return nullptr;

  PAPI_unregister_thread();

  return nullptr;
}

int main(int argc, const char **argv)
{
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <path_to_hwloc_xml>\n";
    return EXIT_FAILURE;
  }

  sys_sage::Node node;
  if (sys_sage::parseHwlocOutput(&node, argv[1]) != 0)
    return EXIT_FAILURE;

  int rval;

  rval = PAPI_library_init(PAPI_VER_CURRENT);
  if (rval != PAPI_VER_CURRENT)
    FATAL(PAPI_strerror(rval));

  rval = PAPI_thread_init(pthread_self);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  int eventSet = PAPI_NULL;
  rval = PAPI_create_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  int events[] = {
    PAPI_TOT_INS,
    PAPI_TOT_CYC
  };
  int numEvents = sizeof(events) / sizeof(events[0]);
  rval = PAPI_add_events(eventSet, events, numEvents);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  PAPI_option_t opt;
  opt.cpu.eventset = eventSet;
  opt.cpu.cpu_num = hwThreadId;
  rval = PAPI_set_opt(PAPI_CPU_ATTACH, &opt);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  pthread_attr_t attr;
  rval = pthread_attr_init(&attr);
  if (rval != 0)
    FATAL(strerror(rval));

  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);
  CPU_SET(hwThreadId, &cpuSet);
  rval = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuSet);
  if (rval != 0)
    FATAL(strerror(rval));

  pthread_t worker;
  worker_args wargs {.topoRoot = &node, .eventSet = eventSet};
  rval = pthread_create(&worker, &attr, work, reinterpret_cast<void *>( &wargs ));
  if (rval != 0)
    FATAL(strerror(rval));

  pthread_join(worker, nullptr);
  pthread_attr_destroy(&attr);

  if (wargs.rval != PAPI_OK)
    FATAL(PAPI_strerror(wargs.rval));

  assert(wargs.outThread->GetId() == hwThreadId);

  wargs.outThread->PrintPAPICounters();

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));
 
  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  PAPI_shutdown();

  return EXIT_SUCCESS;
}

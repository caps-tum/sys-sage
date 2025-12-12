/*
 * This example showcases the (limited) multithreading support of sys-sage PAPI.
 * As of now, concurrent performance monitoring is only guaranteed to work
 * for threads that are pinned to different CPUs. This is due to the sys-sage
 * topology not being thread-safe in general.
 *
 * In this example, we...
 *
 *   - ...initialize PAPI with multithreading support.
 *   - ...attach event sets to different CPUs.
 *   - ...spawn worker threads that are pinned to their respective CPUs.
 *   - ...do performance monitoring within the worker threads.
 *   - ...assert that sys-sage captured the correct CPUs and print the
 *        respective perf counters in the main thread.
 */

#include "sys-sage.hpp"
#include <assert.h>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <set>
#include <stddef.h>
#include <stdlib.h>
#include <sched.h>

#define FATAL(errMsg) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  return EXIT_FAILURE;\
} while (false)

struct worker_args {
  sys_sage::Component *root;
  sys_sage::Relation *metrics = nullptr;
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
  auto warg = reinterpret_cast<worker_args *>( arg );

  warg->rval = PAPI_register_thread();
  if (warg->rval != PAPI_OK)
    return nullptr;

  size_t n = 1'000'000;
  auto a = std::make_unique<double[]>(n);
  auto b = std::make_unique<double[]>(n);
  auto c = std::make_unique<double[]>(n);
  double alpha = 3.14159;

  warg->rval = sys_sage::SS_PAPI_start(warg->eventSet, &warg->metrics);
  if (warg->rval != PAPI_OK)
    return nullptr;

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  warg->rval = sys_sage::SS_PAPI_stop(warg->metrics, warg->root);
  if (warg->rval != PAPI_OK)
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

  const std::set<sys_sage::Component *> cpus {
    node.GetSubcomponentById(1, sys_sage::ComponentType::Thread), // CPU 1
    node.GetSubcomponentById(3, sys_sage::ComponentType::Thread), // CPU 3
    node.GetSubcomponentById(5, sys_sage::ComponentType::Thread), // CPU 5
    node.GetSubcomponentById(7, sys_sage::ComponentType::Thread), // CPU 7
  };

  int rval;
  size_t s;

  rval = PAPI_library_init(PAPI_VER_CURRENT);
  if (rval != PAPI_VER_CURRENT)
    FATAL(PAPI_strerror(rval));

  // enable multithreading support
  rval = PAPI_thread_init(pthread_self);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  int eventSets[cpus.size()];
  for (auto &eventSet : eventSets) {
    eventSet = PAPI_NULL;
    rval = PAPI_create_eventset(&eventSet);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  int events[] = {
    PAPI_TOT_INS,
    PAPI_TOT_CYC
  };
  int numEvents = sizeof(events) / sizeof(events[0]);

  char eventNames[numEvents][PAPI_MAX_STR_LEN] = { { '\0' } };
  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], eventNames[i]);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  for (auto eventSet : eventSets) {
    rval = PAPI_add_events(eventSet, events, numEvents);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  // attach event sets to the CPUs given by `cpuIds`
  PAPI_option_t opts[cpus.size()];
  for (s = 0; auto cpu : cpus) {
    opts[s].cpu.eventset = eventSets[s];
    opts[s].cpu.cpu_num = cpu->GetId();
    rval = PAPI_set_opt(PAPI_CPU_ATTACH, &opts[s]);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));

    s++;
  }

  // pin worker threads to the CPUs
  pthread_attr_t attrs[cpus.size()];
  cpu_set_t cpuSets[cpus.size()];
  for (s = 0; auto cpu : cpus) {
    rval = pthread_attr_init(&attrs[s]);
    if (rval != 0)
      FATAL(strerror(rval));

    CPU_ZERO(&cpuSets[s]);
    CPU_SET(cpu->GetId(), &cpuSets[s]);
    rval = pthread_attr_setaffinity_np(&attrs[s], sizeof(cpu_set_t), &cpuSets[s]);
    if (rval != 0)
      FATAL(strerror(rval));

    s++;
  }

  // spawn worker threads
  pthread_t workers[cpus.size()];
  worker_args wargs[cpus.size()];
  for (s = 0; s < cpus.size(); s++) {
    wargs[s].root = &node;
    wargs[s].eventSet = eventSets[s];

    rval = pthread_create(&workers[s], &attrs[s], work, reinterpret_cast<void *>( &wargs[s] ));
    if (rval != 0)
      FATAL(strerror(rval));
  }

  for (s = 0; s < cpus.size(); s++) {
    pthread_join(workers[s], nullptr);
    pthread_attr_destroy(&attrs[s]);

    if (wargs[s].rval != PAPI_OK)
      FATAL(PAPI_strerror(wargs[s].rval));

    rval = PAPI_cleanup_eventset(eventSets[s]);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
 
    rval = PAPI_destroy_eventset(&eventSets[s]);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  for (s = 0; auto cpu : cpus) {
    // make sure that sys-sage captured the correct CPU
    assert(wargs[s].metrics->GetComponents().size() == 1
           && wargs[s].metrics->GetComponent(0) == cpu);

    // print perf counter values
    std::cout << "metrics on CPU " << cpu->GetId() << ":\n";
    for (int i = 0; i < numEvents; i++)
      std::cout << "  " << eventNames[i] << ": " << wargs[s].metrics->GetPAPImetric(events[i], cpu->GetId()) << '\n';

    s++;
  }

  PAPI_shutdown();

  return EXIT_SUCCESS;
}

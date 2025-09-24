#include <sys-sage.hpp>
#include <omp.h>
#include <papi.h>
#include <iostream>
#include <stdlib.h>

#define FATAL(errMsg) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  return EXIT_FAILURE;\
} while (false)

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

  rval = PAPI_thread_init(omp_get_thread_num);
  if (rval != PAPI_VER_CURRENT)
    FATAL(PAPI_strerror(rval));

  #pragma omp parallel
  {
    int rval;

    rval = PAPI_register_thread();
    if (rval != PAPI_OK);
      // TODO: cancel parallel region

    int eventSet = PAPI_NULL;
    rval = PAPI_create_eventset(&eventSet);
    if (rval != PAPI_OK);
      // TODO: cancel parallel region

    int events[] = {
      PAPI_TOT_INS,
      PAPI_TOT_CYC
    };
    int numEvents = sizeof(events) / sizeof(events[0]);
    rval = PAPI_add_events(eventSet, events, numEvents);
    if (rval != PAPI_OK);
      // TODO: cancel parallel region

    sys_sage::Thread *thread;

    rval = PAPI_start(eventSet);
    if (rval != PAPI_OK);
      // TODO: cancel parallel region

    // TODO: do work with purposeful load imbalance

    rval = sys_sage::PAPI_stop(eventSet, &node, &thread);
    if (rval != PAPI_OK);
      // TODO: cancel parallel region

    thread->PrintPAPICounters();

    rval = PAPI_cleanup_eventset(eventSet);
    if (rval != PAPI_OK);
      // TODO: cancel parallel region

    rval = PAPI_destroy_eventset(&eventSet);
    if (rval != PAPI_OK);
      // TODO: cancel parallel region
  }

  return EXIT_SUCCESS;
}

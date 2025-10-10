#include "sys-sage.hpp"
#include <papi.h>
#include <iostream>
#include <memory>
#include <stdlib.h>

#define FATAL(errMsg) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  return EXIT_FAILURE;\
} while (false)

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
  for (size_t i = 0; i < n; i++)
    a[i] = alpha * b[i] + c[i];
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

  size_t n = 1'000'000;
  auto a = std::make_unique<double[]>(n);
  auto b = std::make_unique<double[]>(n);
  auto c = std::make_unique<double[]>(n);
  double alpha = 3.14159;

  rval = PAPI_library_init(PAPI_VER_CURRENT);
  if (rval != PAPI_VER_CURRENT)
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

  char eventNames[numEvents][PAPI_MAX_STR_LEN] = { { '\0' } };
  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], eventNames[i]);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  unsigned long long timestamps[3] = { 0 };
  sys_sage::Thread *thread;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_read(eventSet, &node, &timestamps[0], &thread);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  std::cout << "reading performance counters on thread " << thread->GetId() << ":\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << thread->GetPAPICounterReading(eventNames[i], timestamps[0]).value_or(-1) << '\n';

  rval = PAPI_reset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  for (int i = 0; i < 5; i++) {
    saxpy(a.get(), b.get(), c.get(), n, alpha);
    rval = sys_sage::PAPI_accum(eventSet, &node, &timestamps[1], &thread);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  std::cout << "accumulating performance counters on thread " << thread->GetId() << ":\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << thread->GetPAPICounterReading(eventNames[i], timestamps[1]).value_or(-1) << '\n';

  rval = PAPI_reset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_stop(eventSet, &node, &timestamps[2], &thread);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  std::cout << "reading performance counters on thread " << thread->GetId() << ":\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << thread->GetPAPICounterReading(eventNames[i], timestamps[2]).value_or(-1) << '\n';

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));
 
  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  PAPI_shutdown();

  return EXIT_SUCCESS;
}

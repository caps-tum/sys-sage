#include "sys-sage.hpp"
#include <papi.h>
#include <iostream>
#include <memory>
#include <stdlib.h>

static constexpr int iter = 5;

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
  sys_sage::PAPIMetrics *metrics = nullptr;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_read(eventSet, timestamps, &node, &metrics);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  rval = PAPI_reset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  for (int i = 0; i < iter; i++) {
    saxpy(a.get(), b.get(), c.get(), n, alpha);
    rval = sys_sage::PAPI_accum(eventSet, timestamps + 1, &node, &metrics);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  long long counters[numEvents];
  saxpy(a.get(), b.get(), c.get(), n, alpha);

  // use plain PAPI interchangeably with the sys-sage integration
  rval = PAPI_read(eventSet, counters);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  rval = PAPI_reset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_stop(eventSet, timestamps + 2, &node, &metrics);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  std::cout << "performance counters on the 1st reading:\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << metrics->GetPerfCounterReading(events[i], timestamps[0]) << '\n';

  std::cout << "accumulated performance counters:\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << metrics->GetPerfCounterReading(events[i], timestamps[1]) << '\n';

  std::cout << "performance counters not stored in the sys-sage topology:\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << counters[i] << '\n';

  std::cout << "performance counters on the last reading:\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << metrics->GetPerfCounterReading(events[i], timestamps[2]) << '\n';

  std::cout << "performance counters were monitored on HW thread(s):";
  for (sys_sage::Component *c : metrics->GetComponents())
    std::cout << ' ' << static_cast<sys_sage::Thread *>(c)->GetId();
  std::cout << '\n';

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));
 
  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  PAPI_shutdown();

  return EXIT_SUCCESS;
}

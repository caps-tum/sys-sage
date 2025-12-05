#include "sys-sage.hpp"
#include <papi.h>
#include <iostream>
#include <memory>
#include <stdlib.h>

static constexpr int ITER = 3;

#define FATAL(errMsg) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  return EXIT_FAILURE;\
} while (false)

void printResults(int *events, const char (*eventNames)[PAPI_MAX_STR_LEN],
                  int numEvents, sys_sage::Relation *metrics)
{
  std::cout << "total perf counter vals:\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << sys_sage::GetCpuPerfVal(metrics, events[i]) << '\n';

  std::cout << "\nperf counters per CPUs:\n";
  for (const sys_sage::Component *cpu : metrics->GetComponents()) {
    int cpuNum = cpu->GetId();
    std::cout << "  CPU " << cpuNum << ":\n";

    for (int i = 0; i < numEvents; i++) {
      std::cout << "    " << eventNames[i] << ":\n";
      for (const sys_sage::PerfEntry &perfEntry : sys_sage::GetCpuPerf(metrics, events[i], cpuNum)->perfEntries)
        std::cout << "      " << perfEntry << '\n';
    }
  }
}

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

  sys_sage::Relation *metrics = nullptr;
  rval = sys_sage::SS_PAPI_start(eventSet, &metrics);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  for (int i = 0; i < ITER; i++) {
    saxpy(a.get(), b.get(), c.get(), n, alpha);

    rval = sys_sage::SS_PAPI_read(metrics, &node, true);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval));
  }

  // stop the event set without storing perf counters -> use plain PAPI_stop
  rval = PAPI_stop(eventSet, nullptr);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));
 
  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  PAPI_shutdown();

  printResults(events, eventNames, numEvents, metrics);

  return EXIT_SUCCESS;
}

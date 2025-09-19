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
  const char *hwlocXml = argv[1];
  sys_sage::Node *node = new sys_sage::Node();
  if (sys_sage::parseHwlocOutput(node, hwlocXml) != 0)
    return EXIT_FAILURE;

  int rval;
  size_t n = 1'000'000;
  std::unique_ptr<double[]> a = std::make_unique<double[]>(n);
  std::unique_ptr<double[]> b = std::make_unique<double[]>(n);
  std::unique_ptr<double[]> c = std::make_unique<double[]>(n);
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

  sys_sage::Thread *thread;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_stop(eventSet, node, &thread);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  thread->PrintPAPICounters();

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  return EXIT_SUCCESS;
}

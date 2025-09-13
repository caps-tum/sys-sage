#include "sys-sage.hpp"
#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
  for (size_t i = 0; i < n; i++)
    a[i] = alpha * b[i] + c[i];
}

int main(int argc, const char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s <path_to_hwloc_xml>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  const char *hwlocXml = argv[1];
  sys_sage::Node *node = new sys_sage::Node();
  if (sys_sage::parseHwlocOutput(node, hwlocXml) != 0) {
    fprintf(stderr, "could not parse topology in file '%s'\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  int rval;
  const size_t n = 1'000'000;
  std::unique_ptr<double[]> a = std::make_unique<double[]>(n);
  std::unique_ptr<double[]> b = std::make_unique<double[]>(n);
  std::unique_ptr<double[]> c = std::make_unique<double[]>(n);
  double alpha = 3.14159;

  rval = PAPI_library_init(PAPI_VER_CURRENT);
  if (rval != PAPI_VER_CURRENT) {
    fprintf(stderr, "%s\n", PAPI_strerror(rval));
    return EXIT_FAILURE;
  }

  int eventSet = PAPI_NULL;
  rval = PAPI_create_eventset(&eventSet);
  if (rval != PAPI_OK) {
    fprintf(stderr, "%s\n", PAPI_strerror(rval));
    return EXIT_FAILURE;
  }

  int events[] = {
    PAPI_TOT_INS,
    PAPI_TOT_CYC
  };
  int numEvents = sizeof(events) / sizeof(events[0]);
  rval = PAPI_add_events(eventSet, events, numEvents);
  if (rval != PAPI_OK) {
    fprintf(stderr, "%s\n", PAPI_strerror(rval));
    return EXIT_FAILURE;
  }

  sys_sage::Thread *thread;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK) {
    fprintf(stderr, "%s\n", PAPI_strerror(rval));
    return EXIT_FAILURE;
  }

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_stop(eventSet, node, &thread);
  if (rval != PAPI_OK) {
    fprintf(stderr, "%s\n", PAPI_strerror(rval));
    return EXIT_FAILURE;
  }

  thread->PrintPAPICounters();
  printf("%d\n", thread->GetId());

  return EXIT_SUCCESS;
}

#include "sys-sage.hpp"
#include <assert.h>
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>

void migrate(int targetCpu)
{
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);
  CPU_SET(targetCpu, &cpuSet);

  if (pthread_setaffinity_np(pthread_self(), sizeof(cpuSet), &cpuSet))
    exit(EXIT_FAILURE);

  sched_yield();
  
  int cpu = sched_getcpu();
  assert(cpu == targetCpu);
}

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
  for (size_t i = 0; i < n; i++)
    a[i] = alpha * b[i] + c[i];
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <path_to_hwloc_xml>\n";
    return EXIT_FAILURE;
  }

  sys_sage::Node node;
  if (sys_sage::parseHwlocOutput(&node, argv[1]) != 0)
    return EXIT_FAILURE;

  size_t n = 1'000'000;
  auto *a = new double[n];
  auto *b = new double[n];
  auto *c = new double[n];
  double alpha = 3.14159;

  int numCpus = get_nprocs();
  int cpu = sched_getcpu();
  int targetCpu = (cpu + 1) % numCpus;
  int targetTargetCpu = (targetCpu + 1) % numCpus;

  PAPI_library_init(PAPI_VER_CURRENT);

  int eventSet = PAPI_NULL;
  PAPI_create_eventset(&eventSet);
  PAPI_add_event(eventSet, PAPI_TOT_INS);

  sys_sage::Relation *metrics = nullptr;
  sys_sage::SS_PAPI_start(eventSet, &metrics);

  saxpy(a, b, c, n, alpha);
  sys_sage::SS_PAPI_read(metrics, &node);
  migrate(targetCpu);

  saxpy(a, b, c, n, alpha);
  sys_sage::SS_PAPI_read(metrics, &node);
  migrate(targetTargetCpu);

  saxpy(a, b, c, n, alpha);
  sys_sage::SS_PAPI_read(metrics, &node);
  migrate(cpu);

  saxpy(a, b, c, n, alpha);
  sys_sage::SS_PAPI_read(metrics, &node);

  PAPI_stop(eventSet, nullptr);

  assert(metrics->GetComponents().size() == 3);

  const sys_sage::CpuPerf *cpuPerf = sys_sage::GetCpuPerf(metrics, PAPI_TOT_INS, cpu);
  assert(cpuPerf->perfEntries.size() == 1);
  std::cout << "CPU " << cpu << ": " << sys_sage::GetCpuPerfVal(metrics, PAPI_TOT_INS, cpu) << '\n';

  const sys_sage::CpuPerf *targetCpuPerf = sys_sage::GetCpuPerf(metrics, PAPI_TOT_INS, targetCpu);
  assert(targetCpuPerf->perfEntries.size() == 1);
  std::cout << "CPU " << targetCpu << ": " << sys_sage::GetCpuPerfVal(metrics, PAPI_TOT_INS, targetCpu) << '\n';

  const sys_sage::CpuPerf *targetTargetCpuPerf = sys_sage::GetCpuPerf(metrics, PAPI_TOT_INS, targetTargetCpu);
  assert(targetTargetCpuPerf->perfEntries.size() == 1);
  std::cout << "CPU " << targetTargetCpu << ": " << sys_sage::GetCpuPerfVal(metrics, PAPI_TOT_INS, targetTargetCpu) << '\n';

  std::cout << "\ntotal: " << sys_sage::GetCpuPerfVal(metrics, PAPI_TOT_INS) << '\n';

  return EXIT_SUCCESS;
}

#include <papi.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

void perf_read(int eventSet, long long &counter, long long &tmp)
{
  PAPI_read(eventSet, &tmp);
  printf("actual val: %lld\n", tmp);
  counter = tmp;
  printf("PAPI output: %lld\n", counter);
}

void perf_accum(int eventSet, long long &counter, long long &tmp)
{
  tmp = 0;
  PAPI_accum(eventSet, &tmp);
  printf("actual val: %lld\n", tmp);
  counter += tmp;
  printf("PAPI output: %lld\n", counter);
}

void migrate(int target_cpu)
{
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(target_cpu, &cpu_set);

  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set), &cpu_set))
    exit(EXIT_FAILURE);

  sched_yield();
  
  int cpu = sched_getcpu();
  assert(cpu == target_cpu);
  printf("\nre-scheduled on CPU: %d\n", cpu);
}

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
  for (size_t i = 0; i < n; i++)
    a[i] = alpha * b[i] + c[i];
}

int main()
{
  size_t n = 1'000'000;
  auto *a = new double[n];
  auto *b = new double[n];
  auto *c = new double[n];
  double alpha = 3.14159;

  long long counter;
  long long tmp = 0;

  int num_cpus = get_nprocs();
  int cpu = sched_getcpu();
  int target_cpu = (cpu + 1) % num_cpus;
  int target_target_cpu = (target_cpu + 1) % num_cpus;

  printf("current CPU: %d, target CPU: %d, target target cpu: %d\n", cpu, target_cpu, target_target_cpu);

  PAPI_library_init(PAPI_VER_CURRENT);

  int eventSet = PAPI_NULL;
  PAPI_create_eventset(&eventSet);
  PAPI_add_event(eventSet, PAPI_TOT_INS);

  PAPI_start(eventSet);

  saxpy(a, b, c, n, alpha);
  perf_read(eventSet, counter, tmp);
  migrate(target_cpu);

  saxpy(a, b, c, n, alpha);
  perf_read(eventSet, counter, tmp);
  migrate(target_target_cpu);

  saxpy(a, b, c, n, alpha);
  perf_read(eventSet, counter, tmp);
  migrate(cpu);

  saxpy(a, b, c, n, alpha);
  perf_read(eventSet, counter, tmp);

  return EXIT_SUCCESS;
}

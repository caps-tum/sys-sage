#include "sys-sage.hpp"
#include <papi.h>
#include <errno.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace sys_sage;

#define FATAL(errMsg, pid) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  kill(pid, SIGKILL);\
  return EXIT_FAILURE;\
} while(false)

void printResults(int *events, const char (*eventNames)[PAPI_MAX_STR_LEN],
                  int numEvents, PAPIMetrics *metrics)
{
  std::cout << "total perf counter vals:\n";
  for (int i = 0; i < numEvents; i++)
    std::cout << "  " << eventNames[i] << ": " << metrics->GetCpuPerfVal(events[i]) << '\n';

  std::cout << "\nperf counters per CPUs:\n";
  for (const Component *cpu : metrics->GetComponents()) {
    int cpuNum = cpu->GetId();
    std::cout << "  CPU " << cpuNum << ":\n";

    for (int i = 0; i < numEvents; i++) {
      std::cout << "    " << eventNames[i] << ":\n";
      for (const PerfEntry &perfEntry : metrics->GetCpuPerf(events[i], cpuNum)->perfEntries)
        std::cout << "      " << perfEntry << '\n';
    }
  }
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <path_to_hwloc_xml> <binary_to_execute> [params_for_binary]\n";
    return EXIT_FAILURE;
  }

  Node node;
  if (parseHwlocOutput(&node, argv[1]) != 0)
    return EXIT_FAILURE;

  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "error: " << strerror(errno) << '\n';
    return EXIT_FAILURE;
  } else if (pid == 0) {
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execvp(argv[2], argv + 2);
    return EXIT_FAILURE;
  }

  int rval, status;

  waitpid(pid, &status, 0);
  ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACEEXIT);

  rval = PAPI_library_init(PAPI_VER_CURRENT);
  if (rval != PAPI_VER_CURRENT)
    FATAL(PAPI_strerror(rval), pid);

  int eventSet = PAPI_NULL;
  rval = PAPI_create_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  int events[] = {
    PAPI_TOT_INS,
    PAPI_TOT_CYC
  };
  int numEvents = sizeof(events) / sizeof(events[0]);
  rval = PAPI_add_events(eventSet, events, numEvents);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  char eventNames[numEvents][PAPI_MAX_STR_LEN] = { { '\0' } };
  for (int i = 0; i < numEvents; i++) {
    rval = PAPI_event_code_to_name(events[i], eventNames[i]);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval), pid);
  }

  rval = PAPI_attach(eventSet, pid);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  PAPIMetrics *metrics = nullptr;
  rval = SS_PAPI_start(eventSet, &metrics);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  ptrace(PTRACE_CONT, pid, nullptr, nullptr);

  waitpid(pid, &status, 0);

  if ( !(WIFSTOPPED(status) && (status >> 16) == PTRACE_EVENT_EXIT) )
    FATAL("expected child process to stop right before exit\n", pid);

  rval = SS_PAPI_stop(eventSet, metrics, &node);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  ptrace(PTRACE_CONT, pid, nullptr, nullptr);

  waitpid(pid, &status, 0);

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK) {
    std::cerr << "error: " << PAPI_strerror(rval) << '\n';
    return EXIT_FAILURE;
  }
 
  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK) {
    std::cerr << "error: " << PAPI_strerror(rval) << '\n';
    return EXIT_FAILURE;
  }

  PAPI_shutdown();

  printResults(events, eventNames, numEvents, metrics);

  return EXIT_SUCCESS;
}

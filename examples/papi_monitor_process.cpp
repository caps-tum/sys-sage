/*
 * This example showcases performance monitoring of a child process through
 * sys-sage PAPI.
 * 
 * In this example, we...
 *
 *   - initialize PAPI in the parent process.
 *   - attach an event set to the child process.
 *   - make the child process run the program given by the command line args
 *     of the parent process.
 *   - let the parent process repeatedly take performance measurements of the
 *     child process.
 *   - print the perf counters.
 */

#include "sys-sage.hpp"
#include <errno.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define FATAL(errMsg, pid) do {\
  std::cerr << "error: " << (errMsg) << '\n';\
  kill(pid, SIGKILL);\
  return EXIT_FAILURE;\
} while(false)

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <path_to_hwloc_xml> <binary_to_execute> [params_for_binary]\n";
    return EXIT_FAILURE;
  }

  sys_sage::Node node;
  if (sys_sage::parseHwlocOutput(&node, argv[1]) != 0)
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

  // attach to child process
  rval = PAPI_attach(eventSet, pid);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  sys_sage::Relation *metrics = nullptr;
  rval = sys_sage::SS_PAPI_start(eventSet, &metrics);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval), pid);

  ptrace(PTRACE_CONT, pid, nullptr, nullptr);

  // take repeated measurements
  while (true) {
    usleep(1000);

    waitpid(pid, &status, WNOHANG);
    if (WIFSTOPPED(status) && (status >> 16) == PTRACE_EVENT_EXIT)
      break;

    rval = sys_sage::SS_PAPI_read(metrics, &node, true);
    if (rval != PAPI_OK)
      FATAL(PAPI_strerror(rval), pid);
  }

  ptrace(PTRACE_CONT, pid, nullptr, nullptr);

  waitpid(pid, &status, 0);

  rval = PAPI_stop(eventSet, nullptr);
  if (rval != PAPI_OK) {
    std::cerr << "error: " << PAPI_strerror(rval) << '\n';
    return EXIT_FAILURE;
  }

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

  metrics->PrintAllPAPImetrics();

  PAPI_shutdown();

  return EXIT_SUCCESS;
}

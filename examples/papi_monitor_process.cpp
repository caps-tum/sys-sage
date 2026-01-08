/*
 * This example showcases performance monitoring of a child process through
 * sys-sage PAPI.
 * 
 * In this example, we...
 *
 *   - ...initialize PAPI in the parent process.
 *   - ...attach an event set to the child process.
 *   - ...make the child process run the program given by the command line args
 *        of the parent process.
 *   - ...let the parent process repeatedly take performance measurements of
 *        the child process.
 *   - ...plot the performance counters collected at each CPU.
 */

#include "sys-sage.hpp"
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <sstream>
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

const char *scriptContent = R"PY(
from matplotlib import pyplot as plt
import sys

path = sys.argv[1]
xVals = [ list(map(float, grouping.split(","))) for grouping in sys.argv[2].split(";") ]
yVals = [ list(map(float, grouping.split(","))) for grouping in sys.argv[3].split(";") ]
labels = sys.argv[4].split(";")

for i, (x, y) in enumerate(zip(xVals, yVals)):
    plt.plot(x, y, marker = "o", label = labels[i])

plt.xlabel("time in [ns]")
plt.ylabel("counter value")
plt.legend()
plt.savefig(path + ".png", dpi = 300, bbox_inches = "tight")
plt.close()
)PY";

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
    usleep(100);

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

  // plot the measurements taken on each CPU

  std::filesystem::path plotDir = std::filesystem::weakly_canonical(std::filesystem::absolute(argv[0]).parent_path()) / "plots";
  std::error_code err;
  bool created = std::filesystem::create_directory(plotDir, err);
  if (err) {
    std::cerr << "error: could not create directory: " << err.message() << '\n';
    return EXIT_FAILURE;
  }

  if (created) {
    std::cout << "created directory " << plotDir << '\n';
  } else {
    for (const auto &file : std::filesystem::directory_iterator(plotDir))
      std::filesystem::remove_all(file.path());
  }

  std::filesystem::path tmpScript = std::filesystem::temp_directory_path() / "plotScriptXXXXXX.py";
  std::string tmpScriptName = tmpScript.string();
  int fd = mkstemps(tmpScriptName.data(), 3);
  if (fd == -1) {
    std::cerr << "error: could not create " << tmpScriptName << '\n';
    return EXIT_FAILURE;
  }
  close(fd);

  {
    std::ofstream script (tmpScriptName);
    script << scriptContent;
  }

  // create a graph for each CPU
  char buf[PAPI_MAX_STR_LEN];
  for (auto cpu : metrics->GetComponents()) {
    // create a timeline that starts relative to the start of the event set
    std::ostringstream xVals;
    std::ostringstream yVals;
    std::ostringstream labels;

    for (int i = 0; i < numEvents; i++) {
      if (i > 0) {
        xVals << ";";
        yVals << ";";
        labels << ";";
      }

      rval = PAPI_event_code_to_name(events[i], buf);
      if (rval != PAPI_OK) {
        std::cerr << "error: " << PAPI_strerror(rval) << '\n';
        return EXIT_FAILURE;
      }
      labels << buf;

      // get all collected metrics of the event on the CPU
      auto cpuMetrics = metrics->GetAllPAPImetrics(events[i], cpu->GetId());
      for (auto it = cpuMetrics->entries.begin(); it != cpuMetrics->entries.end(); it++) {
        if (it != cpuMetrics->entries.begin()) {
          xVals << ",";
          yVals << ",";
        }

        xVals << metrics->GetElapsedTime(it->timestamp);
        yVals << it->value;
      }
    }

    std::string cmd = "python3 " + tmpScriptName
                    + " '" + (plotDir / ("CPU" + std::to_string(cpu->GetId()))).string()
                    + "' '" + xVals.str()
                    + "' '" + yVals.str()
                    + "' '" + labels.str() + "'";

    system(cmd.c_str());
  }

  std::cout << "plots written to " << plotDir << '\n';
  std::filesystem::remove(tmpScript);

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

  return EXIT_SUCCESS;
}

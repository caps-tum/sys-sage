#ifdef PAPI_METRICS

#include <tuple>
#include <vector>

namespace sys_sage {

  struct PerfEntry {
    unsigned long long timestamp;
    long long value;
    bool permanent;
  };

  class PAPIMetrics : public Relation {
  public:
    PAPIMetrics();

    long long GetPerfCounterReading(int event, unsigned long long timestamp = 0);

    std::vector<PerfEntry> *GetPerfCounterReadings(int event, int cpuNum);

    void PrintLatestPerfCounterReadings();

    bool reset = false;
  };

  int SS_PAPI_start(int eventSet, PAPIMetrics **metrics);

  int SS_PAPI_read(int eventSet, unsigned long long *timestamp, Component *root,
                   PAPIMetrics *metrics);

  int SS_PAPI_reset(int eventSet, PAPIMetrics *metrics);

  int SS_PAPI_accum(int eventSet, unsigned long long *timestamp, Component *root,
                    PAPIMetrics *metrics);

  int SS_PAPI_stop(int eventSet, unsigned long long *timestamp, Component *root,
                   PAPIMetrics *metrics);
}

#endif

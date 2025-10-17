#ifdef PAPI_METRICS

#include <vector>
#include <utility>

namespace sys_sage {
  class PAPIMetrics : public Relation {
  public:
    PAPIMetrics(Thread *thread);

    template <bool accum>
    int StorePerfCounters(const int *events, int numEvents,
                          const long long *counters, 
                          unsigned long long *timestamp);

    long long GetPerfCounterReading(int event, unsigned long long timestamp = 0);

    std::vector<std::pair<unsigned long long, long long>> *
    GetPerfCounterReadings(int event);

    void PrintLatestPerfCounterReadings();
  };

  int PAPI_read(int eventSet, unsigned long long *timestamp, Component *root,
                PAPIMetrics **metrics);

  int PAPI_accum(int eventSet, unsigned long long *timestamp, Component *root,
                 PAPIMetrics **metrics);

  int PAPI_stop(int eventSet, unsigned long long *timestamp, Component *root,
                PAPIMetrics **metrics);
}

#endif

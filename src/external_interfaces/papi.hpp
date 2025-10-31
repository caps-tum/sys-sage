#ifdef PAPI_METRICS

#include <tuple>
#include <vector>

namespace sys_sage {

  struct PerfEntry {
    unsigned long long timestamp;
    long long value;
    bool permanent;
  };
  
  struct CpuPerf {
    std::vector<PerfEntry> perfEntries;
    int cpuNum;
  };

  // the relation class doesn't fit my needs entirely:
  //   - it would be better to use an `attrib` map that uses integer keys
  //     for the event codes instead of `std::string`
  //   - the `components` vector needs to support reference counting

  // TODO: change name to `CpuMetrics`
  class PAPIMetrics : public Relation {
  public:
    PAPIMetrics();

    long long GetCpuPerfVal(int event, int cpuNum = -1, unsigned long long timestamp = 0);

    CpuPerf *GetCpuPerf(int event, int cpuNum);

    void RemoveCpu(int cpuNum);

    //void PrintLatestPerfCounterReadings();

    std::unordered_map<int, int> cpuReferenceCounters;
    unsigned long long latestTimestamp;
    bool reset;
  };

  int SS_PAPI_start(int eventSet, PAPIMetrics **metrics);

  int SS_PAPI_reset(int eventSet, PAPIMetrics *metrics);

  template <bool stop = false>
  int SS_PAPI_read(int eventSet, PAPIMetrics *metrics, Component *root,
                   bool permanent = false,
                   unsigned long long *timestamp = nullptr);

  int SS_PAPI_accum(int eventSet, PAPIMetrics *metrics, Component *root,
                    bool permanent = false,
                    unsigned long long *timestamp = nullptr);
}

#endif

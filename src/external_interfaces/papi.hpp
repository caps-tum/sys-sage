#ifdef PAPI_METRICS

#ifndef SRC_EXTERNAL_INTERFACES_PAPI_HPP
#define SRC_EXTERNAL_INTERFACES_PAPI_HPP

#include <unordered_map>
#include <vector>
#include <ostream>

namespace sys_sage {

  /*
   * @brief An object representing a single perf counter value.
   */
  struct PerfEntry {
    unsigned long long timestamp;
    long long value;
    bool permanent;
  };
  
  /*
   * @brief An object representing multiple perf counter values of a single CPU.
   */
  struct CpuPerf {
    std::vector<PerfEntry> perfEntries;
    int cpuNum;
  };

  /*
   * @brief A sys-sage relation used to capture PAPI metrics.
   */
  class PAPI_Metrics : public Relation {
  public:
    PAPI_Metrics();

    /*
     * @brief sys-sage wrapper around `PAPI_reset`.
     *
     * @param eventSet The event set to be resetted.
     *
     * @return The same as with `PAPI_reset`.
     */
    int PAPI_reset(int eventSet);

    /*
     * @brief sys-sage wrapper around `PAPI_read`.
     *
     * @param eventSet The event set to read from.
     * @param root The root of the hardware topology.
     * @param permanent An optional flag indicating whether the perf counter
     *                  values should be stored permanently or treated as
     *                  temporary (i.e. can be overwritten).
     * @param timestamp An optional output paramter containing the timestamp of the
     *                  perf counter reading.
     *
     * @return The return-codes of PAPI have been used. For more info, have a look at PAPI's documentation.
     */
    int PAPI_read(int eventSet, Component *root, bool permanent = false,
                  unsigned long long *timestamp = nullptr);

    /*
     * @brief sys-sage wrapper around `PAPI_accum`.
     *
     * @param eventSet The event set to read from.
     * @param root The root of the hardware topology.
     * @param permanent An optional flag indicating whether the perf counter
     *                  values should be stored permanently or treated as
     *                  temporary (i.e. can be overwritten).
     * @param timestamp An optional output paramter containing the timestamp of the
     *                  perf counter reading.
     *
     * @return The return-codes of PAPI have been used. For more info, have a look at PAPI's documentation.
     */
    int PAPI_accum(int eventSet, Component *root, bool permanent = false,
                   unsigned long long *timestamp = nullptr);

    /*
     * @brief sys-sage wrapper around `PAPI_stop`.
     *
     * @param eventSet The event set to read from.
     * @param root The root of the hardware topology.
     * @param permanent An optional flag indicating whether the perf counter
     *                  values should be stored permanently or treated as
     *                  temporary (i.e. can be overwritten).
     * @param timestamp An optional output paramter containing the timestamp of the
     *                  perf counter reading.
     *
     * @return The return-codes of PAPI have been used. For more info, have a look at PAPI's documentation.
     */
    int PAPI_stop(int eventSet, Component *root, bool permanent = false,
                  unsigned long long *timestamp = nullptr);

    /*
     * @brief Get the perf counter value of a specific event.
     *
     * @param event The event of interest.
     * @param cpuNum An optional parameter used to filter out the perf counter
     *               value of a single CPU. If the value is -1, the perf
     *               counter value of all CPUs combined is provided.
     * @param timestamp An optional parameter used to filter out the perf
     *        counter value belonging to a specific perf counter reading. A
     *        value of 0 refers to the latest reading.
     *
     * @return > 0 if a perf counter value exists for the given paramters, 0 otherwise.
     */
    long long GetCpuPerfVal(int event, int cpuNum = -1, unsigned long long timestamp = 0);

    /*
     * @brief Get all perf counter values of a specific event and CPU.
     *
     * @param event The event of interest.
     * @param cpuNum The CPU of interest.
     *
     * @return A valid pointer to an object containing the perf counter values,
     *         if such an object exists for the given paramters, nullptr otherwise.
     */
    CpuPerf *GetCpuPerf(int event, int cpuNum);

    void S__Reset();

  private:
    void DeletePerfEntries();
    void RemoveCpu(int);

    int StorePerfCounters(const int *, int, const long long *, Thread *, bool,
                          unsigned long long *);
    int AccumPerfCounters(const int *, int, const long long *, Thread *, bool,
                          unsigned long long *);

    std::unordered_map<int, int> cpuReferenceCounters;
    unsigned long long latestTimestamp;
    bool reset;
  };

  /*
   * @brief sys-sage wrapper around `PAPI_start`.
   *
   * @param eventSet The event set to be started.
   * @param metrics An output parameter that can point to a PAPI_Metrics
   *                relation. The pointer must not be nullptr. If the pointer
   *                that is being pointed to is nullptr, a new PAPIMetrics
   *                relation is created and `*metrics` is set accordingly.
   *                Otherwise, `*metrics` is reused.
   *
   * @return The same as with `PAPI_start`, with the addition that if `metrics == nullptr`,
   *         `PAPI_EINVAL` is returned.
   */
  int PAPI_start(int eventSet, PAPI_Metrics **metrics);

}

/*
 * @brief Enables easy printing for objects of type `PerfEntry`.
 */
std::ostream &operator<<(std::ostream &stream, const sys_sage::PerfEntry &perfEntry);

#endif // SRC_EXTERNAL_INTERFACES_PAPI_HPP

#endif // PAPI_METRICS

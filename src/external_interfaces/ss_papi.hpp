#ifdef PAPI

#ifndef SRC_EXTERNAL_INTERFACES_SS_PAPI_HPP
#define SRC_EXTERNAL_INTERFACES_SS_PAPI_HPP

#include <papi.h>
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
   * @brief sys-sage wrapper around `PAPI_start`.
   *
   * @param eventSet The event set to be started.
   * @param metrics An output parameter that can point to a valid Relation
   *                object of category `RelationCategory::PAPI_Metrics`. The
   *                pointer must not be `nullptr`. If the pointer that is being
   *                pointed to is `nullptr`, a new Relation object of category
   *                `RelationCategory::PAPI_Metrics` is created and `*metrics`
   *                is set accordingly. Otherwise, `*metrics` is reused.
   *
   * @return If `metrics == nullptr` or `*metrics` is not `nullptr` and not of
   *         category `RelationCategory::PAPI_Metrics`, then `PAPI_EINVAL` is
   *         returned, otherwise the same as in `PAPI_start`.
   *
   */
  int SS_PAPI_start(int eventSet, Relation **metrics);

  /*
   * @brief sys-sage wrapper around `PAPI_reset`.
   *
   * @param metrics The relation of category `PAPI_Metrics` that represents the
   *                eventSet.
   *
   * @return If `metrics == nullptr` or `metrics` is not of category
   *         `RelationCategory::PAPI_Metrics`, then `PAPI_EINVAL` is returned,
   *          otherwise the same as in `PAPI_reset`.
   */
  int SS_PAPI_reset(Relation *metrics);

  /*
   * @brief sys-sage wrapper around `PAPI_read`.
   *
   * @param metrics The relation of category `RelationCategory::PAPI_Metrics`
   *                that represents the eventSet.
   * @param root The root of the hardware topology.
   * @param permanent An optional flag indicating whether the perf counter
   *                  values should be stored permanently or treated as
   *                  temporary (i.e. can be overwritten).
   * @param timestamp An optional output paramter containing the timestamp of the
   *                  perf counter reading.
   *
   * @return The return-codes of PAPI have been used. For more info, have a look at PAPI's documentation.
   */
  int SS_PAPI_read(Relation *metrics, Component *root, bool permanent = false,
                   unsigned long long *timestamp = nullptr);

  /*
   * @brief sys-sage wrapper around `PAPI_accum`.
   *
   * @param metrics The relation of category `RelationCategory::PAPI_Metrics`
   *                that represents the eventSet.
   * @param root The root of the hardware topology.
   * @param permanent An optional flag indicating whether the perf counter
   *                  values should be stored permanently or treated as
   *                  temporary (i.e. can be overwritten).
   * @param timestamp An optional output paramter containing the timestamp of the
   *                  perf counter reading.
   *
   * @return The return-codes of PAPI have been used. For more info, have a look at PAPI's documentation.
   */
  int SS_PAPI_accum(Relation *metrics, Component *root, bool permanent = false,
                    unsigned long long *timestamp = nullptr);

  /*
   * @brief sys-sage wrapper around `PAPI_stop`.
   *
   * @param metrics The relation of category `RelationCategory::PAPI_Metrics`
   *                that represents the eventSet.
   * @param root The root of the hardware topology.
   * @param permanent An optional flag indicating whether the perf counter
   *                  values should be stored permanently or treated as
   *                  temporary (i.e. can be overwritten).
   * @param timestamp An optional output paramter containing the timestamp of the
   *                  perf counter reading.
   *
   * @return The return-codes of PAPI have been used. For more info, have a look at PAPI's documentation.
   */
  int SS_PAPI_stop(Relation *metrics, Component *root, bool permanent = false,
                   unsigned long long *timestamp = nullptr);

  /*
   * @brief Get the perf counter value of a specific event.
   *
   * @param metrics The relation of category `RelationCategory::PAPI_Metrics`
   *                that represents the eventSet.
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
  long long GetCpuPerfVal(const Relation *metrics, int event, int cpuNum = -1,
                          unsigned long long timestamp = 0);


  /*
   * @brief Get all perf counter values of a specific event and CPU.
   *
   * @param metrics The relation of category `RelationCategory::PAPI_Metrics`
   *                that represents the eventSet.
   * @param event The event of interest.
   * @param cpuNum The CPU of interest.
   *
   * @return A valid pointer to an object containing the perf counter values,
   *         if such an object exists for the given paramters, nullptr otherwise.
   */
  const CpuPerf *GetCpuPerf(const Relation *metrics, int event, int cpuNum);

}

/*
 * @brief Enables easy printing for objects of type `PerfEntry`.
 */
std::ostream &operator<<(std::ostream &stream, const sys_sage::PerfEntry &perfEntry);

#endif // SRC_EXTERNAL_INTERFACES_SS_PAPI_HPP

#endif // PAPI

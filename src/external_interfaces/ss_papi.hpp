#ifdef SS_PAPI

#ifndef SRC_EXTERNAL_INTERFACES_SS_PAPI_HPP
#define SRC_EXTERNAL_INTERFACES_SS_PAPI_HPP

#include <papi.h>
#include <unordered_map>
#include <vector>
#include <ostream>

/*! \file */

namespace sys_sage {

  /**
   * @brief An object representing a single perf counter value.
   */
  struct Metric {
    unsigned long long timestamp;
    long long value;
    bool permanent;
  };

  /**
   * @brief An object collecting multiple performance metrics of a single CPU.
   */
  struct CpuMetrics {
    std::vector<Metric> entries;
    int cpuNum;
  };

  /**
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

  /**
   * @brief sys-sage wrapper around `PAPI_reset`.
   *
   * @param metrics The relation of category `RelationCategory::PAPI_Metrics`
   *                that represents the eventSet.
   *
   * @return If `metrics == nullptr` or `metrics` is not of category
   *         `RelationCategory::PAPI_Metrics`, then `PAPI_EINVAL` is returned,
   *          otherwise the same as in `PAPI_reset`.
   */
  int SS_PAPI_reset(Relation *metrics);

  /**
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

  /**
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

  /**
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

}

/**
 * @brief Enables easy printing for objects of type `Metric`.
 */
std::ostream &operator<<(std::ostream &stream, const sys_sage::Metric &metric);

#endif // SRC_EXTERNAL_INTERFACES_SS_PAPI_HPP

#endif // SS_PAPI

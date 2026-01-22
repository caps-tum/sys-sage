#ifndef MT4G_PARSER
#define MT4G_PARSER

#include "sys-sage.hpp"
#include <string>

namespace sys_sage {

  /*
   * @brief Construct a complete GPU topology by parsing an mt4g output file.
   *        The topology will be represented by a newly created Chip component.
   *
   * @param parent The parent of the newly created Chip component.
   * @param path The path to the mt4g output file.
   * @param gpuId - The ID used for the Chip component.
   *
   * @return 0 on success, 1 on failure.
   */
  int ParseMt4g(Component *parent, const std::string &path, int gpuId);

  /*
   * @brief Construct a complete GPU topology by parsing an mt4g output file.
   *        The topology will be represented by a given Chip component.
   *
   * @param gpu The Chip component used to represent the topology.
   * @param path The path to the mt4g output file.
   *
   * @return 0 on success, 1 on failure.
   */
  int ParseMt4g(Chip *gpu, const std::string &path);

  /*
   * @brief Construct a complete GPU topology by parsing an mt4g output file.
   *        The topology will be represented by a newly created Chip component.
   *        Equivalent to `ParseMt4g`.
   *
   * @param parent The parent of the newly created Chip component.
   * @param path The path to the mt4g output file.
   * @param gpuId - The ID used for the Chip component.
   *
   * @return 0 on success, 1 on failure.
   */
  int ParseMt4g_v1_x(Component *parent, const std::string &path, int gpuId);

  /*
   * @brief Construct a complete GPU topology by parsing an mt4g output file.
   *        The topology will be represented by a given Chip component.
   *        Equivalent to `ParseMt4g`.
   *
   * @param gpu The Chip component used to represent the topology.
   * @param path The path to the mt4g output file.
   *
   * @return 0 on success, 1 on failure.
   */
  int ParseMt4g_v1_x(Chip *gpu, const std::string &path);

  /*
   * @brief Construct a complete GPU topology by parsing an mt4g output file.
   *        The topology will be represented by a newly created Chip component.
   *        This parser is made for version v0.1 of mt4g and is not compatible
   *        with version v1.x and above.
   *
   * @param parent The parent of the newly created Chip component.
   * @param path The path to the mt4g output file.
   * @param gpuId - The ID used for the Chip component.
   * @param delim (default ";") The delimiter used in the CSV file.
   *
   * @return 0 on success, 1 on failure.
   */
  int ParseMt4g_v0_1(Component* parent, const std::string &path, int gpuId, const std::string delim = ";");

  /*
   * @brief Construct a complete GPU topology by parsing an mt4g output file.
   *        The topology will be represented by a given Chip component.
   *        This parser is made for version v0.1 of mt4g and is not compatible
   *        with version v1.x and above.
   *
   * @param gpu The Chip component used to represent the topology.
   * @param path The path to the mt4g output file.
   * @param delim (default ";") The delimiter used in the CSV file.
   *
   * @return 0 on success, 1 on failure.
   */
  int ParseMt4g_v0_1(Chip* gpu, const std::string &path, const std::string delim = ";");

}

#endif

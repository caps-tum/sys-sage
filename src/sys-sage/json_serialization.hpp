#ifndef SYS_SAGE_SRC_JSON_SERIALIZATION_HPP
#define SYS_SAGE_SRC_JSON_SERIALIZATION_HPP

#include <sys-sage/Component.hpp>
#include <sys-sage/Relation.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <unordered_map>
#include <stdint.h>

namespace sys_sage {
    /**
     * @brief Dumps the entire subtree and the corresponding relation graph
     *        spanned by the provided component into a JSON object.
     *
     * @param component The component spanning the subtree.
     * @param obj The JSON object to contain the data.
     */
    void DumpJson(const Component *component, nlohmann::json &obj);

    /**
     * @brief Dumps the entire subtree and the corresponding relation graph
     *        spanned by the provided component into a JSON file.
     *
     * @param component The component spanning the subtree.
     * @param path The path of the output JSON file. If the path is empty,
     *             the JSON data will be dumped into stdout.
     *
     * @return 0 on success, 1 on failure to write to the file.
     */
    int DumpJson(const Component *component, const std::filesystem::path &path = "");

    /**
     * @brief Loads the component tree and the relation graph from a JSON
     *        object.
     *
     * @param obj The JSON object containing the data.
     *
     * @return A pointer to the root of the component tree. May return
     *         `nullptr` on failure.
     */
    Component *LoadJson(const nlohmann::json &obj);

    /**
     * @brief Loads the component tree and the relation graph from a JSON file.
     *
     * @param path The path to the JSON file.
     *
     * @return A pointer to the root of the component tree. May return
     *         `nullptr` on failure.
     */
    Component *LoadJson(const std::filesystem::path &path);

    /**
     * @brief Loads the component tree and the relation graph from a JSON file.
     *        Directs string literals to filesystem paths and avoids ambiguous
     *        overloaded function calls.
     *
     * @param path The path to the JSON file.
     *
     * @return A pointer to the root of the component tree. May return
     *         `nullptr` on failure.
     */
    inline Component *LoadJson(const char *path)
    {
        return LoadJson(std::filesystem::path(path));
    }

    /**
     * @private
     *
     * @brief Provides idiomatic JSON serialization for all components. Used
     *        internally by `nlohmann::json`.
     *
     * @param obj The JSON object to be initialized.
     * @param component The component of interest.
     */
    void to_json(nlohmann::json &obj, const Component *component);

    /**
     * @private
     *
     * @brief Provides idiomatic JSON deserialization for all components. Used
     *        internally by `nlohmann::json`.
     *
     * @param obj The JSON object containing the data.
     * @param component A pointer referencing the initialized component. Will
     *                  be set to `nullptr` on failure.
     */
    void from_json(const nlohmann::json &obj, Component *&component);

    /**
     * @private
     *
     * @brief Provides idiomatic JSON serialization for all relations. Used
     *        internally by `nlohmann::json`.
     *
     * @param obj The JSON object to be initialized.
     * @param relation The relation of interest.
     */
    void to_json(nlohmann::json &obj, const Relation *relation);
}

#endif

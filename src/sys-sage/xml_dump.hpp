#ifndef XML_DUMP
#define XML_DUMP

#include <functional>

#include "Component.hpp"
#include "DataPath.hpp"

namespace sys_sage{
    /**
     * @brief Exports the Component Tree to an XML file.
     *
     * Traverses the component hierarchy starting from the given root and serializes the structure,
     * including all components, their attributes, and relations, into an XML file.
     *
     * @param root Pointer to the root Component of the tree to export.
     * @param path Output file path (if empty, returns the XML tree in memory).
     * @param search_custom_attrib_key_fcn Optional user-provided function for custom attribute serialization (string attributes).
     * @param search_custom_complex_attrib_key_fcn Optional user-provided function for custom attribute serialization (complex attributes, e.g., XML nodes).
     * @return 0 on success, nonzero on error.
     */
    int exportToXml(Component *root, std::string path = "", std::function<int(std::string, void *, std::string *)> search_custom_attrib_key_fcn = NULL, std::function<int(std::string, void *, xmlNodePtr)> search_custom_complex_attrib_key_fcn = NULL);
    /**
     * @private
     * @brief Default handler for complex attribute serialization. Can be used as a reference for creating custom handlers.
     *
     * Used internally for serializing attributes that require custom XML node construction.
     * Users can override this by providing their own function to exportToXml.
     *
     * @param key Attribute key.
     * @param value Pointer to the attribute value.
     * @param n XML node to attach the attribute to.
     * @return 0 on success, nonzero on error.
     */
    int _search_default_complex_attrib_key(std::string key, void* value, xmlNodePtr n);
    /**
     * @private
     * @brief Default handler for simple attribute serialization. Can be used as a reference for creating custom handlers.
     *
     * Used internally for serializing attributes as strings for XML output.
     * Users can override this by providing their own function to exportToXml.
     *
     * @param key Attribute key.
     * @param value Pointer to the attribute value.
     * @param ret_value_str Output: string representation of the value.
     * @return 0 on success, nonzero on error.
     */
    int _search_default_attrib_key(std::string key, void *value, std::string *ret_value_str);
    /**
     * @private
     * @brief Builds the XML subtree for a given component.
     *
     * Recursively serializes the component and its children into an XML subtree.
     * Used internally by exportToXml.
     *
     * @param root Pointer to the root Component.
     * @return Pointer to the created XML node.
     */
    xmlNodePtr _buildComponentSubtree(Component* root);
    /**
     * @private
     * @brief Prints the attributes of a component or relation to XML.
     *
     * Used for debugging or for custom XML output.
     *
     * @param attrib Map of attribute key-value pairs.
     * @param n XML node to attach the attributes to.
     * @return 0 on success, nonzero on error.
     */
    int _print_attrib(std::map<std::string, void *> attrib, xmlNodePtr n);
} //namespace sys_sage
#endif

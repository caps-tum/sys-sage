#ifndef XML_LOAD
#define XML_LOAD

#include <functional>

#include "Component.hpp"
#include "DataPath.hpp"

//SVTODO make sure all functions from the .cpp are also in the header
//SVTODO check the import and export functionalities and adapt them to Relations
namespace sys_sage {
    /**
     * @brief Imports the sys-sage internal representation from an XML file.
     *
     * Parses the XML file at the given path and reconstructs both the Component tree and the Relations graph,
     * including all components, their attributes, and relations.
     *
     * @param path Path to the XML file.
     * @param search_custom_attrib_key_fcn Optional user-provided function for custom attribute deserialization (string attributes).
     * @param search_custom_complex_attrib_key_fcn Optional user-provided function for custom attribute deserialization (complex attributes, e.g., XML nodes).
     * @return Pointer to the root Component of the imported tree.
     */
    Component* importFromXml(std::string path, std::function<void*(xmlNodePtr)> search_custom_attrib_key_fcn = NULL, std::function<int(xmlNodePtr, Component*)> search_custom_complex_attrib_key_fcn = NULL);

    /**
     * @private
     * @brief Extracts a string property from an XML node.
     * @param n XML node pointer.
     * @param prop Name of the property to extract.
     * @return The property value as a string.
     */
    std::string _getStringFromProp(xmlNodePtr n, std::string prop);
    /**
     * @private
     * @brief Creates Relation objects from an XML node.
     * @param relationNode XML node representing the relation(s).
     * @return 0 on success, nonzero on error.
     */   
    int _CreateRelations(xmlNodePtr relationNode);

    /**
     * @private
     * @brief Recursively creates a Component subtree from an XML node.
     * @param n XML node pointer.
     * @return Pointer to the root Component of the created subtree.
     */
    Component* _CreateComponentSubtree(xmlNodePtr n);
    /**
     * @private
     * @brief Searches for and deserializes a default attribute from an XML node. Can be used as a reference for creating custom handlers.
     * Used for simple attribute types.
     * @param n XML node pointer.
     * @return Pointer to the deserialized value (void*).
     */
    void* _search_default_attrib_key(xmlNodePtr n);
    /**
     * @private
     * @brief Searches for and deserializes a complex attribute from an XML node and adds it to a Component. Can be used as a reference for creating custom handlers.
     * Used for attributes that require custom XML node parsing.
     * @param n XML node pointer.
     * @param c Pointer to the Component to attach the attribute to.
     * @return 0 on success, nonzero on error.
     */
    int _search_default_complex_attrib_key(xmlNodePtr n, Component *c);
    /**
     * @private
     * @brief Collects all attributes from an XML node and adds them to a Component.
     * @param n XML node pointer.
     * @param c Pointer to the Component to attach the attributes to.
     * @return 0 on success, nonzero on error.
     */
    int _collect_attrib(xmlNodePtr n, Component* c);
} //namespace sys_sage
#endif


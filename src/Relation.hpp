#ifndef RELATION_HPP
#define RELATION_HPP

/**
 * @file Relation.hpp
 * @brief Defines the Relation class for modeling connections between components in system topologies.
 *
 * This header provides the Relation class, a core component of the sys-sage library.
 * Relations represent interactions or connections between components in a system topology,
 * enabling flexible modeling and analysis of architectural relationships.
 * Derived classes like DataPath, QuantumGate, and CouplingMap inherit from Relation
 * to represent specific types of connections.
 */

#include <map>
#include <vector>
#include <string>
#include <libxml/parser.h>

#include "defines.hpp"
#include "enums.hpp"

namespace sys_sage { //forward declaration
    class Component;
    class Qubit;
    struct CpuMetrics;
}

namespace sys_sage {

    /**
     * @class Relation
     * @brief Abstract base class representing a multi-way connection among Components.
     *
     * A Relation models a relationship among one or more Components, such as a data path,
     * logical gate, or any other connectivity construct. Derived classes define the specific
     * type and semantics.
     *
     * Key Features:
     * - The @c components vector holds participating components. The @c ordered flag
     *   determines whether their order is semantically meaningful.
     * - The @c id and @c name identify the relation.
     * - The @c type indicates the specific kind of relation (e.g. data path, gate).
     * - The class supports extensible metadata via @c attrib.
     *
     * Clients can use this interface generically or extend it with domain-specific semantics.
     */
    class Relation {
    public:
        /**
         * @brief Construct a new Relation object.
         * @param components List of pointers to participating Components.
         * @param _id Optional unique ID for the relation.
         * @param _ordered Whether the order of components carries semantic meaning.
         *
         * The type of the relation is set to sys_sage::RelationType::Relation.
         */
        Relation(const std::vector<Component*>& components, int _id = 0, bool _ordered = true,
                 RelationCategory::type category = RelationCategory::Uncategorized);
        /**
         * @brief Sets the id of the relationship.
         * @param _id The id of the relationship to set.
         */
        void SetId(int _id);
        /**
         * @brief Gets the id of the relationship.
         * @return The current id of the relationship.
         */
        int GetId() const;
        /**
         * @brief Get the type of the relation.
         * @return The current type of the relation (as sys_sage::RelationType::type).
         */
        RelationType::type GetType() const;

        /**
         * @brief Get the category of the relation.
         */
        RelationCategory::type GetCategory() const;

        /**
         * @brief Return a human-readable name of the relation type.
         * @return A string like "DataPath" or "QuantumGate".
         */
        std::string GetTypeStr() const;
        /**
         * @brief Check if this relation treats component order as meaningful.
         * @return True if the order of components matters.
         */
        bool IsOrdered() const;
        /**
         * @brief Check whether the given component is part of this relation.
         * @param c Pointer to the Component to check.
         * @return True if @p c is found in the components vector.
         */
        bool ContainsComponent(Component* c) const;
        /**
         * @brief Get the component at a specific position.
         * @param index Index in the component list.
         * @return Pointer to the Component at that index.
         */
        Component* GetComponent(int index) const;
        /**
         * @brief Access the list of components.
         * @return Read-only reference to the component vector.
         * 
         * This avoids copying and prevents direct modification.
         */
        const std::vector<Component*>& GetComponents() const;
        /**
         * @brief Virtual function to print the details of the relationship.
         * 
         * Derived classes may implement this function to provide specific
         * printing behavior.
         */
        virtual void Print() const;
        /**
         * @private
         * @brief Print all key-value pairs in the attribute map.
         *
         * Meant for debugging or verbose output. Can be overridden.
         */
        void _PrintRelationAttrib() const;
        /**
         * @private
         * @brief Print summary of the components involved.
         *
         * Useful for debugging the structure of the relation.
        */ 
        void _PrintRelationComponentInfo() const;
        /**
         * @brief Add a new component to the relation.
         * @param c Component to append to the internal list.
         */
        void AddComponent(Component* c);
        /**
         * @brief Replace a component at the given index.
         * @param index The index of the component to replace.
         * @param _new_component New component to insert.
         * @return 0 on success, -1 if the index is invalid.
         */
        int UpdateComponent(int index, Component * _new_component);
        /**
         * @brief Replace the first occurrence of a given component.
         *
         * Uses std::find to locate @p _old_component and replaces it with @p _new_component.
         *
         * @param _old_component The component to replace.
         * @param _new_component The replacement component.
         * @return 0 on success, -1 if not found.
         */
        int UpdateComponent(Component* _old_component, Component * _new_component);

        /**
         * @brief Removes the component at the given index.
         * 
         * @param index The index of interest.
         *
         * @return 0 on success, -1 otherwise.
         */
        int RemoveComponent(size_t index);

        /**
         * @private
         * @brief Serialize this relation to XML.
         * @return A libxml node representing the relation.
         *
         * Should normally not be used directly. Used internally for exporting the relation to XML.
         */
        virtual xmlNodePtr _CreateXmlEntry();
        /**
         * @brief Virtual function to delete the relation.
         *
         * Should be overridden in subclasses if custom destruction logic is needed.
         */
        virtual void Delete();//TODO
        /**
         * @brief Destructor for the Relation class.
         * 
         * This is a virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~Relation() = default;

#ifdef SS_PAPI
        /**
         * @brief Get the perf counter value of a specific event and CPU. Only
         *        works if this relation is of category `RelationCategory::PAPI_Metrics`.
         *
         * @param event The event of interest.
         * @param cpuNum An optional parameter used to distinguish between a
         *               CPU-centric view and an EventSet-centric view. If the
         *               value is greater than -1, the perf counter value of
         *               the CPU with the given `cpuNum` is returned. If it is
         *               equal to -1, the output contains the sum of perf
         *               counter values on all CPUs in the relation.
         * @param timestamp An optional parameter used to select a perf counter
         *                  value from a specific perf counter reading. A value
         *                  of 0 refers to the latest reading.
         *
         * @return > 0 if a perf counter value exists for the given paramters, 0 otherwise.
         */
        long long GetPAPImetric(int event, int cpuNum = -1, unsigned long long timestamp = 0) const;

        /**
         * @brief Get all the perf counter values of a specific event that are
         *        collected on a specific CPU. Only works if this relation is
         *        of category `RelationCategory::PAPI_Metrics`.
         *
         * @param event The event of interest.
         * @param cpuNum The CPU of interest.
         *
         * @return A valid pointer to an object containing the perf counter
         *         values. If such an object doesn't exist for the given
         *         paramters, `nullptr` is returned.
         */
        const CpuMetrics *GetAllPAPImetrics(int event, int cpuNum) const;

        /**
         * @brief Print PAPI metrics of all CPUs stored in this relation.
         */
        void PrintPAPImetrics(int cpuNum = -1) const;

        /**
         * @brief Retrieve all PAPI events stored in this relation.
         *
         * @return A vector containing the event codes. It may be empty if no
         *         such events exist.
         */
        std::vector<int> FindPAPIevents() const;

        /**
         * @brief Retrieve all PAPI events stored in this relation.
         *
         * @param events A vector used for storing the event codes.
         */
         void FindPAPIevents(std::vector<int> &events) const;

        /**
         * @brief Returns the time between the given timestamp and the start of
         *        the event set.
         *
         * @param timestamp The timestamp of interest.
         *
         * @return If this relation is not of category
         *         `RelationCategory::PAPI_Metrics`, 0 is returned, otherwise
         *         the elapsed time.
         */
        unsigned long long GetElapsedTime(unsigned long long timestamp) const;
#endif

    protected:
        /**
         * @private
         * @brief Protected constructor for internal use. Makes sure that the relation type is set correctly.
         * @param _relation_type The type of the relation (see RelationType::type).
         */
        Relation(RelationType::type _relation_type, RelationCategory::type _relation_category);
        /**
         * @private
         * @brief Protected constructor for internal use. Makes sure that the relation type is set correctly.
         * @param components List of pointers to participating Components.
         * @param _id Optional unique ID for the relation.
         * @param _ordered Whether the order of components carries semantic meaning.
         * @param _relation_type The type of the relation (see RelationType::type).
         */
        Relation(const std::vector<Component*>& components, int _id, bool _ordered, RelationType::type _relation_type, RelationCategory::type _relation_category);

        /**
         * @brief Whether order in the component list is meaningful.
         */
        bool ordered;
        /**
         * @brief The id of the relationship.
         *
         * This member variable stores the unique identifier for the relationship.
         */
        int id;
        /**
         * @brief The type of the relationship (see RelationType::type).
         *
         * This member variable stores the type of the relationship.
         */
        RelationType::type type;

        /**
         * @brief The category of the relationship.
         */
        RelationCategory::type category;

        /**
         * @brief A vector of components associated with the relationship.
         * 
         * This member variable holds pointers to components that are part of
         * the relationship.
         */
        std::vector<Component*> components;

    public:
        /**
        * A map for storing arbitrary pieces of information or data.
        * - The `key` denotes the name of the attribute.
        * - The `value` points to the data, stored as a `void*`.
        *
        * This data structure is designed to store a wide variety of data types by
        * utilizing pointers to void. Due to its flexibility, it is essential to manage
        * the types and memory allocation/deallocation carefully to avoid issues such
        * as memory leaks or undefined behavior.
        *
        * Usage:
        * 
        * 1. Adding a new key-value pair:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * int* value = new int(42); // Dynamically allocate memory for the value
        * attrib[key] = static_cast<void*>(value); // Store the value in the map
        * ```
        * 
        * 2. Retrieving data from an existing key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* retrievedValue = static_cast<int*>(attrib[key]);
        *     std::cout << "Value: " << *retrievedValue << std::endl;
        * } else {
        *     std::cout << "Key not found." << std::endl;
        * }
        * ```
        * 
        * 3. Checking for the existence of a key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     std::cout << "Key exists." << std::endl;
        * } else {
        *     std::cout << "Key does not exist." << std::endl;
        * }
        * ```
        * 
        * 4. Removing a key-value pair and freeing memory:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* value = static_cast<int*>(attrib[key]);
        *     delete value; // Free the dynamically allocated memory
        *     attrib.erase(key); // Remove the key-value pair from the map
        * }
        * ```
        * 
        * 5. Updating the value for an existing key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* oldValue = static_cast<int*>(attrib[key]);
        *     delete oldValue; // Free the old value
        *     int* newValue = new int(100); // Allocate new value
        *     attrib[key] = static_cast<void*>(newValue); // Update the map
        * }
        * ```
        * 
        * Note:
        * - Proper memory management is crucial when using `void*` pointers. Always ensure
        *   that dynamically allocated memory is freed when no longer needed.
        * - Type safety is not enforced, so it is important to cast pointers to the correct
        *   type when retrieving values from the map.
        */
        std::map<std::string, void*> attrib;
    };

}
#endif //RELATION_HPP

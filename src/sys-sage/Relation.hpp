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
#include <string>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <map>
#include <utility>
#include <libxml/parser.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <stdint.h>

#include <sys-sage/defines.hpp>
#include <sys-sage/enums.hpp>
#include <sys-sage/attribute.hpp>

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
         * @private
         *
         * @brief Default-initializes a new Relation object.
         * @param _id Optional unique ID for the relation.
         * @param _ordered Whether the order of components carries semantic meaning.
         *
         * The type of the relation is set to sys_sage::RelationType::Relation.
         */
        Relation(int _id = 0, bool _ordered = true,
                 RelationCategory::type _category = RelationCategory::Default);
        
        /**
         * @brief Construct a new Relation object.
         * @param components List of pointers to participating Components.
         * @param _id Optional unique ID for the relation.
         * @param _ordered Whether the order of components carries semantic meaning.
         *
         * The type of the relation is set to sys_sage::RelationType::Relation.
         */
        Relation(const std::vector<Component*>& components, int _id = 0, bool _ordered = true,
                 RelationCategory::type _category = RelationCategory::Default);

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy constructor.
         */
        Relation(const Relation &) = delete;

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy
         *        assignment operator.
         */
        Relation &operator=(const Relation &) = delete;

         /**
         * @brief Destructor for relations.
         *        Unlinks this relation from its components and frees resources.
         */
        virtual ~Relation();

        /**
         * @brief Deletes the given relation.
         *        This assumes that the relation is HEAP-ALLOCATED.
         *
         * @param rel The relation to be deleted.
         */
        static void Delete(Relation *rel);

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
        const std::string &GetTypeStr() const;
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
         * @brief Removes the component at the given index. This does not delete the component.
         * 
         * @param index The index of interest.
         *
         * @return 0 on success, -1 otherwise.
         */
        int RemoveComponent(size_t index);

        /**
         * @brief Removes the given component from the relation. This does not delete the component.
         *
         * @param The component of interest.
         *
         * @return 0 on success, -1 otherwise.
         */
        int RemoveComponent(Component *component);

        /**
         * @private
         * @brief Serialize this relation to XML.
         * @return A libxml node representing the relation.
         *
         * Should normally not be used directly. Used internally for exporting the relation to XML.
         */
        virtual xmlNodePtr _CreateXmlEntry();

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this relation.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        virtual void _ToJson(nlohmann::json &obj) const;

        /**
         * @private
         *
         * @brief Initializes this relation through JSON. Intended for internal
         *        use.
         *
         * @param obj The JSON object containing the data.
         * @param componentMap A map used for the relation graph.
         *
         * @return 0 on success, 1 otherwise.
         */
        virtual int _FromJson(const nlohmann::json &obj,
                              const std::unordered_map<uintptr_t, Component *> &componentMap);

#ifdef SS_PAPI
        /**
         * @brief Get the perf counter value of a specific event and CPU. Only
         *        works if this relation is of category `RelationCategory::PAPI_Metrics`.
         *
         * @param eventCode The event of interest.
         * @param cpuNum An optional parameter used to distinguish between a
         *               CPU-centric view and an EventSet-centric view. If the
         *               value is greater than -1, the perf counter value of
         *               the CPU with the given `cpuNum` is returned. If it is
         *               equal to -1, the output contains the sum of perf
         *               counter values on all CPUs in the relation.
         * @param timestamp An optional parameter used to select a perf counter
         *                  value from a specific perf counter reading. A value
         *                  of 0 refers to the latest reading. Note that the
         *                  last recorded metrics entry of a CPU does not have
         *                  to belong to the latest perf counter reading. This
         *                  discrepency can happen when the last entry is set
         *                  to "permanent mode" (see documentation of
         *                  `sys_sage::SS_PAPI_read`) and this CPU was not
         *                  involved in the latest reading.
         *
         * @return > 0 if a perf counter value exists for the given paramters, 0 otherwise.
         */
        long long GetPAPImetric(int eventCode, int cpuNum = -1, unsigned long long timestamp = 0) const;

        /**
         * @brief Get all the perf counter values of a specific event that are
         *        collected on a specific CPU. Only works if this relation is
         *        of category `RelationCategory::PAPI_Metrics`.
         *
         * @param eventCode The event of interest.
         * @param cpuNum The CPU of interest.
         *
         * @return A valid pointer to an object containing the perf counter
         *         values. If such an object doesn't exist for the given
         *         paramters, `nullptr` is returned.
         */
        const CpuMetrics *GetAllPAPImetrics(int eventCode, int cpuNum) const;

        /**
         * @brief Print PAPI metrics measured on the CPUs stored in this
         *        relation. If this relation is not of category
         *        `RelationCategory::PAPI_Metrics`, nothing will be done.
         *
         * @param cpuNum Optional parameter used to decide whether all CPUs
         *        should be printed or only a specific one. If the value is -1,
         *        the former will be done. Otherwise, the value is interpreted
         *        as the target CPUs ID.
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
          * @brief Retrieve the event set to which this relation is currently
          *        (or was previously) bound to.
          *
          * @return The corresponding event set. If this relation is not of
          *         category `RelationCategory::PAPI_Metrics`, then `PAPI_NULL`
          *         will be returned instead.
          */
        int GetCurrentEventSet() const;

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

        /**
         * @brief Retrieve the ID of the CPU on which the latest perf counter
         *        reading was conducted on.
         *
         * @return If this relation is not of category
         *         `RelationCategory::PAPI_Metrics` or no measurements have yet
         *         been taken, -1 is returned. Otherwise, the ID of the
         *         described CPU.
         */
        int GetLatestCpuNum() const;
#endif

        /**
         * @brief Iterator type for attributes iteration.
         */
        using attribIterator = std::map<std::string, std::unique_ptr<IAttribute>>::iterator;

        /**
         * @brief Constant iterator type for attributes iteration.
         */
        using constAttribIterator = std::map<std::string, std::unique_ptr<IAttribute>>::const_iterator;

        /**
         * @brief Size type for estimating the number of stored attributes.
         */
        using attribSizeType = std::map<std::string, std::unique_ptr<IAttribute>>::size_type;

        /**
         * @brief Inserts an attribute using a key-value pair.
         *
         * @param key The key that is associated with the attribute.
         * @param value The value of the attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         new attribute.
         */
        template <typename T>
        std::decay_t<T> *SetAttribute(const std::string &key, T &&value);

        /**
         * @brief Retrieves the stored value of the attribute that is
         *        associated with the given key.
         *
         * @param key The key that is associated with the attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         attribute. May be `nullptr` if no attribute is associated
         *         with the given key or the requested type doesn't match the
         *         stored type.
         */
        template <typename T>
        T *GetAttribute(const std::string &key);

        /**
         * @brief Retrieves the stored value of the constant attribute that is
         *        associated with the given key.
         *
         * @param key The key that is associated with the constant attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         constant attribute. May be `nullptr` if no attribute is
         *         associated with the given key or the requested type doesn't
         *         match the stored type.
         */
        template <typename T>
        const T *GetAttribute(const std::string &key) const;

        /**
         * @brief Retrieves the stored value of the attribute iterator.
         *
         * @param it The iterator of the attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         attribute. May be `nullptr` if no attribute is associated
         *         with the given iterator or the requested type doesn't match
         *         the stored type.
         */
        template <typename T>
        T *GetAttribute(attribIterator it);

        /**
         * @brief Retrieves the stored value of the constant attribute iterator.
         *
         * @param it The iterator of the constant attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         constant attribute. May be `nullptr` if no attribute is
         *         associated with the given iterator or the requested type
         *         doesn't match the stored type.
         */
        template <typename T>
        const T *GetAttribute(constAttribIterator it) const;

        /**
         * @brief Updates an existing attribute that is associated with the
         *        given key with a new value. May reuse existing memory and can
         *        be more resource-efficient. If no such attribute exists, the
         *        key-value pair is used to insert a new one.
         *
         * @param key The key that is associated with the attribute.
         * @param value The new value of the attribute.
         *
         * @return A pointer to the respective object storing the updated value
         *         of the attribute. May be `nullptr` if the type of the new
         *         value does not match the stored type of the existing
         *         attribute. Use the `Relation::SetAttribute` method to
         *         update in this case.
         */
        template <typename T>
        std::decay_t<T> *UpdateAttribute(const std::string &key, T &&value);

        /**
         * @brief Updates an existing attribute that is associated with the
         *        given iterator with a new value. May reuse existing memory
         *        and can be more resource-efficient.
         *
         * @param it The iterator that is associated with the attribute.
         * @param value The new value of the attribute.
         *
         * @return A pointer to the respective object storing the updated value
         *         of the attribute. May be `nullptr` if no attribute is
         *         associated with the given iterator or the type of the new
         *         value does not match the stored type. Use the
         *         `Relation::SetAttribute` method to update in the latter
         *         case.
         */
        template <typename T>
        std::decay_t<T> *UpdateAttribute(attribIterator it, T &&value);

        /**
         * @private
         *
         * @brief Inserts a type-erased attribute.
         *
         * @param key The key associated with the attribute.
         * @param attribute The type-erased attribute.
         */
        // TODO: somehow make this private and find a better name
        void _EmplaceAttribute(const std::string &key, std::unique_ptr<IAttribute> &attribute);

        /**
         * @brief Returns the number of stored attributes.
         *
         * @return The respective size.
         */
        attribSizeType GetAttributesSize() const;

        /**
         * @brief Returns an iterator to the beginning of the attributes.
         *
         * @return The iterator of the first attribute.
         */
        attribIterator AttributesBegin();

        /**
         * @brief Returns an iterator to the beginning of the constant
         *        attributes.
         *
         * @return The iterator of the first constant attribute.
         */
        constAttribIterator AttributesBegin() const;

        /**
         * @brief Returns an iterator to the end of the attributes.
         *
         * @return The iterator of the last attribute.
         */
        attribIterator AttributesEnd();

        /**
         * @brief Returns an iterator to the end of the constant attributes.
         *
         * @return The iterator of the last constant attribute.
         */
        constAttribIterator AttributesEnd() const;

        /**
         * @brief Removes the attribute that is associated to the given key.
         *
         * @param key The key that is associated to the attribute.
         */
        void EraseAttribute(const std::string &key);

        /**
         * @brief Removes the attribute of the given iterator.
         *
         * @param key The iterator of the attribute.
         *
         * @return The iterator to the next attribute.
         */
        attribIterator EraseAttribute(attribIterator it);

        /**
         * @brief Removes all attributes.
         */
        void ClearAttributes();

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
         * @private
         * @brief Protected constructor for internal use. Makes sure that the relation type is set correctly.
         * @param _id Optional unique ID for the relation.
         * @param _ordered Whether the order of components carries semantic meaning.
         * @param _relation_type The type of the relation (see RelationType::type).
         */
        Relation(int _id, bool _ordered, RelationType::type _relation_type, RelationCategory::type _relation_category);

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

        /**
         * @brief The attributes map.
         */
        std::map<std::string, std::unique_ptr<IAttribute>> attributes;
    };

}

#include <sys-sage/Relation.inl>

#endif //RELATION_HPP

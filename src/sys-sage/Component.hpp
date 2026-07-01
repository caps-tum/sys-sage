#ifndef COMPONENT
#define COMPONENT

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

#include <sys-sage/defines.hpp>
#include <sys-sage/enums.hpp>
#include <sys-sage/DataPath.hpp>
#include <sys-sage/attribute.hpp>
#include <libxml/parser.h>
#include <nlohmann/json.hpp>

namespace sys_sage { //forward declaration
    class Topology;

    class Relation;
    class DataPath;
    class QuantumGate;
}


namespace sys_sage {
    //SVTODO make sure parameters such as ComponentType are of the correct type

    /**
     * @class Component
     * @brief Generic class for all hardware and logical components in sys-sage.
     *
     * All components inherit from this class, which defines attributes and methods common to all components.
     * This enables a unified interface for tree traversal, querying, and manipulation.
     * Usually, a Component instance is one of the derived subclasses, but a generic Component is also possible.
     */
    class Component {
    public:
        /**
         * @brief Generic Component constructor (no automatic insertion in the Component Tree).
         * Usually one of the derived subclasses for different Component Types will be created, not this one.
         * @param _id Numeric ID of the component (default 0)
         * @param _name Name of the component (default "unknown")
         *
         * Sets componentType to sys_sage::ComponentType::Generic.
         */
        Component(int _id = 0, const std::string &_name = "unknown");
        /**
         * @brief Generic Component constructor with insertion into the Component Tree as the parent's child.
         * Usually one of the derived subclasses for different Component Types will be created.
         * @param parent Pointer to the parent component
         * @param _id Numeric ID of the component (default 0)
         * @param _name Name of the component (default "unknown")
         *
         * Sets componentType to sys_sage::ComponentType::Generic.
         */
        Component(Component * parent, int _id = 0, const std::string &_name = "unknown");

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy constructor.
         */
        Component(const Component &) = delete;

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy
         *        assignment operator.
         */
        Component &operator=(const Component &) = delete;

        //SVTODO reevaluate the delete vs destructor
        /**
         * @brief Destructor for components.
         *        Unlinks this component from its parent and children and additionally frees resources.
         *        The destructor does not delete the entire subtree.
         *        Refer to `Component::DeleteSubtree()` for the latter.
         */
        virtual ~Component();

        /**
         * @brief Deletes the given component and all relations it is associated with.
         *        This assumes that the component and the relations are all HEAP-ALLOCATED.
         *
         * @param comp The component to be deleted.
         */
        static void Delete(Component *comp);

        /**
         * @brief Deletes the whole subtree spanned by the given component and all associated relations.
         *        This assumes that the components in the subtree and the relations are all HEAP-ALLOCATED.
         *
         * @param root The root of the subtree.
         * @param keepRoot If set to `false`, the given component will be deleted as well.
         */
        static void DeleteSubtree(Component *root, bool keepRoot = false);

        /**
         * @brief Inserts a child component to this component (in the Component Tree).
         * The child pointer will be inserted at the end of the children vector.
         * @param child Pointer to a Component (or any class instance that inherits from Component).
         * @see GetChildren()
         * @see GetChild(int _id)
         */
        void InsertChild(Component * child);
        
        /**
         * @brief Inserts this component between a parent and one of its children.
         * The parent component remains the parent, this Component becomes a new child of the parent, and the specified child becomes this component's child.
         * @param parent The parent component to which this component will be inserted as a child.
         * @param child The child component that will become the child of this component and will remain a descendant of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent. 
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified child.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified child.
         * @return 0 on success; 
         *         1 if the child and parent are not child and parent in the component tree; 
         *         2 if the component tree is corrupt (parent is a parent of child but child is not in the parent's children list); 
         *         3 if the component tree is corrupt (parent is not a parent of child but child is in the parent's children list).
        */
        int InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild);
        
        /**
         * @brief Inserts this component between a parent and a (subset of) its children. 
         * The parent component remains parent, this Component becomes a new child, and the children become parent's grandchildren.
         * @param parent The parent component to which this component will be inserted as a child.
         * @param children A vector of child components that will become the children of this component and the grandchildren of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent. 
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified children.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified children.
         * @return 0 on success
         *        \n 1 on incompatible parent-children components (one or more children are not parent's children); 
         *        \n 2 on corrupt component tree (parent is a parent of child but child is NOT in children list of parent); 
         *        \n 3 on corrupt component tree (parent is NOT a parent of child but child is in children list of parent)
        */
        int InsertBetweenParentAndChildren(Component* parent, std::vector<Component*> children, bool alreadyParentsChild);

        /**
         * @brief Removes the passed component from the list of children, without completely deleting (and deallocating) the child itself.
         *        The child's parent pointer will be set to `nullptr`.
         * @param child Child to remove
         * @return Number of elements deleted (normally 0 or 1)
         */
        int RemoveChild(Component * child);
        /**
         * @brief Set a parent to the component.
         * This is usually used when inserting a component in the tree (by calling InsertChild on the parent, and calling SetParent on the child).
         * @param parent Pointer to a Component (or any class instance that inherits from Component).
         * @see InsertChild()
         */
        void SetParent(Component* parent);
        /**
         * @brief Prints the whole subtree of this component (including the component itself) to stdout.
         * The tree is printed in DFS order, so that the hierarchy can be easily seen. Each child is indented by "  ".
         * For each component in the subtree, the following is printed: "<string component type> (name <name>) id <id> - children: <num children>
         */
        void PrintSubtree() const;

        /**
         * @private
         * @brief Helper function for PrintSubtree(), ensures proper indentation.
         * Using PrintSubtree() is recommended, but this can be used for custom indentation.
         * @param level Number of "  " to print before the component.
         * @see PrintSubtree()
         */
    // TODO: search for other parts that should be private
    private:
        void _PrintSubtree(int level) const;
    public:
        /**
         * @brief Prints all Relations in the subtree.
         * @param RelationType Filter by relation type (default: Any)
         */
        void PrintRelationsInSubtree(RelationType::type relationType = RelationType::Any);

        /**
         * @brief Returns name of the component.
         * @return Name
         * @see name
         */
        const std::string& GetName() const;
        /**
         * @brief Sets name of the component.
         * @param _name Name of the component
         * @see name
         */
        void SetName(const std::string &_name);
        /**
         * @brief Returns id of the component.
         * @return id
         * @see id
         */
        int GetId() const;

        /**
         * @brief Sets Id of the component.
         * @param _id Id of the component
         * @see id
         */
        void SetId(int _id);

        /**
         * @brief Returns component type of the component.
         * The component type denotes which class the instance is (often stored as Component*, even though they are a member of one of the child classes).
         * @return componentType (of type sys_sage::ComponentType::type)
         * @see componentType
         */
        sys_sage::ComponentType::type GetComponentType() const;
        /**
         * @brief Returns component type as a human-readable string, as defined in ComponentType::names.
         * @return String representation of the component type.
         * @see componentType
         */
        const std::string &GetComponentTypeStr() const;
        /**
         * @brief Returns a reference to const std::vector containing all children of the component (empty vector if no children).
         * @return const std::vector<Component *> & with children
         */
        const std::vector<Component*>& GetChildren() const;
        /**
         * @private
         * @brief Returns a reference to the non-const children vector (internal use).
         */
        std::vector<Component*>& _GetChildren();
        /**
         * @brief Returns Component pointer to parent (or NULL if this component is the root)
         */
        Component* GetParent() const;
        /**
        * @brief Retrieve a Component* to a child with child.id=_id.
        * Identical to GetChildById
        * \n Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
        * @see GetChildById
        */
        Component* GetChild(int _id) const;

        /**
        * @brief Retrieve a Component* to a child with child.id=_id.
        * Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
        */
        Component* GetChildById(int _id) const;

        /**
        * @brief Retrieve a Component* to a child matching the given component type.
        * Should there be more children with the same type, the first match will be retrieved (i.e. the one with lower index in the children array.)
        * @param _componentType Component type to match
        * @return Pointer to the first matching child, or nullptr if not found
        */
        Component* GetChildByType(ComponentType::type _componentType) const;

        /**
         * @brief Searches for all children matching the given component type.
         * @param _componentType Required type of components
         * @return Vector of all matching children
         */
        std::vector<Component*> FindChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Searches for all the children matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void FindChildrenByType(std::vector<Component *> &_outArray, ComponentType::type _componentType) const;

        /**
        * @brief Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
        * @return Returns first occurence that matches these criteria.
        * @param _id - the id to look for
        * @param _componentType - the component type where to look for the id
        * @return Component * matching the criteria. Returns the first match. NULL if no match found
        */
        Component *GetDescendantById(int _id, ComponentType::type _componentType);

        /**
         * @brief Searches for all the descendants matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void FindDescendantsByType(std::vector<Component*> &outArray, ComponentType::type _componentType);

        /**
         * @brief Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components.
         * @returns A vector of all the subcomponents matching the _componentType.
        */
        std::vector<Component*> FindDescendantsByType(ComponentType::type _componentType);

        /**
         * @brief Counts number of descendants matching the requested component type.
         * @param _componentType - ComponentType to look for.
         * @return Returns number of descendants matching the requested component type.
         */
        int CountDescendantsByType(ComponentType::type _componentType) const;

        /**
        * @brief Counts number of children matching the requested component type.
        * @param _componentType - ComponentType to look for.
        * @return Returns number of children matching the requested component type.
        */
        int CountChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Moves up the tree until a parent of the given type is found.
         * @param _componentType Desired component type
         * @return Pointer to the ancestor, or nullptr if not found
         */
        Component* GetAncestorByType(ComponentType::type _componentType);

        /**
         * @brief Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
         * 0=leaf, 1=children are leaves, 2=at most children's children are leaves .....
         * @return maximal distance to a leaf
         */
        int CalcSubtreeDepth() const;//0=empty, 1=1element,...

        /**
         * @brief Retrieves Nth ancestor, which resides N levels above.
         * E.g. if n=1, the parent is retrieved; if n=2, the grandparent is retrieved and so on.
         * @param n - how many levels above the tree should be looked.
         * @returns The ancestor residing N levels above.
        */
        Component* GetNthAncestor(int n);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
         */
        void FindNthDescendants(std::vector<Component*> &outArray, int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. 
         * The tree is traversed in order as the children are stored in the std::vector.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @return A std::vector<Component*> with the results.
         */
        std::vector<Component*> FindNthDescendants(int depth);

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/Relation::Delete() to modify the list of Relations.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        const std::vector<Relation*>& GetRelationsByType(RelationType::type relationType) const;

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/Relation::Delete() to modify the list of Relations.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        std::vector<Relation*>& _GetRelationsByType(RelationType::type relationType) const;

        /**
         * @brief Returns a newly-constructed vector of all relations of a given type and position.
         * @param relationType Type of relation (default: Any)
         * @param thisComponentPosition Position of this component in the relation (default: -1 = do NOT care about position)
         * @return Vector of matching relations (copy, not reference)
         * @see getRelations(RelationType::type relationType) const as an alternative that returns a reference to the internal structure, i.e. has less overhead.
         */
        std::vector<Relation*> FindRelations(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) const;

        /**
         * @private
         * @brief Only called by Relation's AddComponent/UpdateComponent.
         * @param relationType Type of relation
         * @param r Pointer to the relation
         */
        void _AddRelation(RelationType::type relationType, Relation* r);

        /**
         * @brief Retrieves a DataPath* from the list of this component's data paths with matching DataPathCategory and DataPathDirection.
         * The first match is returned.
         * @param dp_category DataPath category to search for
         * @param direction Orientation (default: Any)
         * @return Pointer to the found DataPath, or nullptr if not found
         */
        DataPath* GetDataPathByCategory(DataPathCategory::type dp_category, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation.
         * Results are returned in std::vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
         * @param outDpArr - output parameter (vector with results)
         * An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
         * The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         * @param dp_category DataPath type to search for (default: Any)
         * @param direction Orientation/direction of a DataPath (default: Any)
         */
        void FindDataPaths(std::vector<DataPath*> &outDpArr, DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation/direction.
         * Results are returned in a std::vector<DataPath*>*.
         * @param dp_category DataPath type to search for (default: Any)
         * @param direction Orientation (default: Any)
         * @return Vector of matching DataPaths
         */
        std::vector<DataPath*> FindDataPaths(DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
        @brief Checks the consistency of the component tree starting from this component.

        This function verifies that each child component has this component set as its parent.
        It logs an error message for each child that has an incorrect parent and increments the error count.
        The function then recursively checks the consistency of the entire subtree rooted at each child component.

        @return The total number of inconsistencies found in the component tree.
        
        The function returns the total number of errors found in the component tree, including errors in the direct children and any nested descendants.
        */
        int CheckSubtreeConsistency() const;

        /**
         * @brief Calculates approximate memory footprint of the subtree of this element (including the relevant Relations).
         * @param out_component_size output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param out_dataPathSize output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @return The total size in bytes
         */
        int CalcSubtreeSize(unsigned * out_component_size, unsigned * out_dataPathSize) const;

        /**
         * @private
         * Helper function of int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize); -- normally you would call this one.
         * \n Calculates approximate memory footprint of the subtree of this element (including the relevant data paths). Does not count DataPaths stored in counted_dataPaths.
         * @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param counted_dataPaths - std::set<DataPath*>* of data paths that should not be counted
         * @return The total size in bytes
         * @see GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
         */
        int _CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*> &countedRelations) const;

        /**
         * @brief Retrieves the depth (level) of a component in the topology.
         * @param refresh If true, recalculate the position (depth) of the component in the tree; if false, return the already stored value
         * @return Depth (level) of the component in the topology
         * @see depth
         */
        int CalcDepth(bool refresh);

        /**
         * @private
         * @brief Helper for XML dump generation.
         * Should normally not be used directly. Used internally for exporting the topology to XML.
         * @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL)
         * @return Pointer to the created XML subtree node.
         */
        virtual xmlNodePtr _CreateXmlSubtree();

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this component.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        virtual void _ToJson(nlohmann::ordered_json &obj) const;

        /**
         * @private
         *
         * @brief Initializes this component through JSON. Intended for
         *        internal use.
         *
         * @param obj The JSON object containing the data.
         *
         * @return 0 on success, 1 otherwise.
         */
        virtual int _FromJson(const nlohmann::ordered_json &obj);

        /**
         * @brief Deletes all relations of this component (optionally filtered by type).
         *        This assumes that all the relations are HEAP-ALLOCATED.
         *
         * @param relationType Relation type to delete (default: Any)
         */
        void DeleteRelations(RelationType::type relationType = RelationType::Any);

#ifdef SS_PAPI
        /**
         * @brief Prints all PAPI metrics of the given event set within the
         *        subtree spanned by this component.
         *
         * @param eventSet Specifies the event set of interest. If the value is
         *        `PAPI_NULL`, then all event sets will be printed.
         */
        void PrintPAPImetricsInSubtree(int eventSet = -1) const;

        /**
         * @brief Retrieves all relations used for collecting PAPI metrics
         *        within the subtree spanned by this component.
         *
         * @return A vector containing said relations.
         */
        std::vector<Relation *> FindPAPIrelationsInSubtree() const;

        /**
         * @brief Retrieves all relations used for collecting PAPI metrics
         *        within the subtree spanned by this component.
         *
         * @param papiRelations A vector used for storing said relations.
         */
        void FindPAPIrelationsInSubtree(std::vector<Relation *> &papiRelations) const;
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
         *         attribute. Use the `Component::SetAttribute` method to
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
         *         `Component::SetAttribute` method to update in the latter
         *         case.
         */
        template <typename T>
        std::decay_t<T> *UpdateAttribute(attribIterator it, T &&value);

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
         * @brief Protected constructor for derived classes (no automatic insertion in the Component Tree).
         * @param _id Numeric ID of the component
         * @param _name Name of the component
         * @param _componentType Component type (of type sys_sage::ComponentType::type)
         */
        Component(int _id, const std::string &_name, ComponentType::type _componentType);

        /**
         * @brief Protected constructor for derived classes with insertion into the Component Tree.
         * @param parent Pointer to the parent component
         * @param _id Numeric ID of the component
         * @param _name Name of the component
         * @param _componentType Component type (of type sys_sage::ComponentType::type)
         */
        Component(Component * parent, int _id, const std::string &_name, ComponentType::type _componentType);

        int id; /**< Numeric ID of the component. There is no requirement for uniqueness of the ID, however it is advised to have unique IDs at least in the realm of parent's children (siblings). Some tree search functions, which take the id as a search parameter search for first match, so the user is responsible to manage uniqueness in the realm of the search subtree (or should be aware of the consequences of not doing so). Component's ID is set by the constructor, and is retrieved via int GetId(); */
        int depth; /**< Depth (level) of the Component in the Component Tree */
        std::string name; /**< Name of the component (as a std::string). */
        int count{-1}; /**< Can be used to represent multiple Components with the same properties. By default, it represents only 1 component, and is set to -1. */
        /**
        Component type of the component. The component type denotes of which class the instance is (often the components are stored as Component*, even though they are a member of one of the child classes)
        Component type is constant, set by constructor, readonly. 
        It can be of types as listed in ComponentType::type (which is user-extensible).*/
        const ComponentType::type componentType;
        std::vector<Component*> children; /**< Contains the list (std::vector) of pointers to children of the component in the component tree. */
        Component* parent { nullptr }; /**< Contains pointer to the parent component in the component tree. If this component is the root, parent will be nullptr.*/
        
        /**
         * Contains a list (std::array) of different Relation types. 
         * Initially nullptr, it is allocated on the first call to AddRelation() or new Relation().
         * The array size is RelationType::_num_relation_types, which is defined in RelationType.
         * Each element of the array is a pointer to a std::vector<Relation*> that contains all Relations of that type. (also lazy-allocated)
         */
        std::array<std::vector<Relation*>*, RelationType::_num_relation_types>* relations = nullptr;

        /**
         * The attributes map.
         */
        std::map<std::string, std::unique_ptr<IAttribute>> attributes;
    };

} //namespace sys_sage 

#include <sys-sage/Component.inl>

#endif //COMPONENT_HPP

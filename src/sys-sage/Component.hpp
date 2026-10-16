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

/**
 * @namespace sys_sage
 *
 * @brief Global namespace of the sys-sage library.
 */
namespace sys_sage {
    /**
     * @class Component
     * @brief Generic class for all hardware and logical components.
     *
     * All components inherit from this class, which defines attributes and methods common to all components.
     * This enables a unified interface for tree traversal, querying, and manipulation.
     * Usually, a Component instance is one of the derived subclasses, but a generic Component is also possible and can for instance be used to model custom hardware components.
     */
    class Component {
    public:
        /**
         * @brief Generic Component constructor (no automatic insertion in the Component Tree).
         *        Sets the componentType member to `sys_sage::ComponentType::Generic`.
         *
         * @param _id Numeric ID of the component (default 0).
         * @param _name Name of the component (default "unknown").
         *
         * @pythonBinding
         */
        Component(int _id = 0, const std::string &_name = "unknown");

        /**
         * @brief Generic Component constructor with insertion into the Component Tree as the parent's child.
         *        Sets componentType to sys_sage::ComponentType::Generic.
         *
         * @param _parent Pointer to the parent component.
         * @param _id Numeric ID of the component (default 0).
         * @param _name Name of the component (default "unknown").
         *
         * @pythonBinding
         */
        Component(Component *_parent, int _id = 0, const std::string &_name = "unknown");

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy constructor.
         */
        Component(const Component &) = delete;

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy
         *        assignment operator.
         */
        Component &operator=(const Component &) = delete;

        /**
         * @brief Allow default move constructor.
         */
        Component(Component &&) = default;

        /**
         * @brief Allow default move assignment operator.
         */
        Component &operator=(Component &&) = default;

        /**
         * @brief Destructor for components.
         *        Unlinks this component from its parent and children and additionally frees resources.
         *        The destructor does not delete the entire subtree.
         *        Refer to sys_sage::Component::DeleteSubtree() for the latter.
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
         * @brief Inserts a new child at the end of the sys_sage::Component::children vector and sets this component as the child's parent.
         *
         * @param child Pointer to a component.
         *
         * @see GetChildren()
         * @see GetChild(int _id)
         *
         * @pythonBinding
         */
        void InsertChild(Component *child);
        
        /**
         * @brief Inserts this component between a parent and one of its children.
         *        The parent component remains the parent, this component becomes a new child of the parent, and the specified child becomes this component's child.
         *
         * @param parent The parent component to which this component will be inserted as a new child.
         * @param child The child component that will become the child of this component and will remain a descendant of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent.
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified child.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified child.
         *
         * @return 0 on success;
         *         \n 1 if the child and parent are not actually child and parent in the Component Tree;
         *         \n 2 if the Component Tree is corrupt (parent is a parent of child but child is not in the parent's children vector);
         *         \n 3 if the Component Tree is corrupt (parent is not a parent of child but child is in the parent's children vector).
         *
         * @pythonBinding
        */
        int InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild);
        
        /**
         * @brief Inserts this component between a parent and a (subset of) its children.
         *        The parent component remains parent, this component becomes a new child, and the specified children become the parent's grandchildren.
         *
         * @param parent The parent component to which this component will be inserted as a child.
         * @param children A vector of child components that will become the children of this component and the grandchildren of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent.
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified children.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified children.
         *
         * @return 0 on success;
         *        \n 1 on incompatible parent-children components (one or more children are not the parent's children);
         *        \n 2 on corrupt Component Tree (parent is a parent of the specified children but at least on of them is not in the parernt's children vector);
         *        \n 3 on corrupt Component Tree (parent is not a parent of at least one of the specified children but the children are in the parent's children vector).
         *
         * @pythonBinding
        */
        int InsertBetweenParentAndChildren(Component* parent, std::vector<Component*> children, bool alreadyParentsChild);

        /**
         * @brief Removes the passed component from the sys_sage::Component::children, without completely deleting (and deallocating) the child itself.
         *        The child's parent pointer will be set to `nullptr`.
         *
         * @param child Child to remove.
         *
         * @return Number of elements removed (normally 0 or 1).
         *
         * @pythonBinding
         */
        int RemoveChild(Component *child);

        /**
         * @brief Set the parent of this component.
         *        This does not add this component to the children vector of the given parent.
         *        Call sys_sage::Component::InsertChild on the parent instead if this is desired.
         *
         * @param _parent The new parent.
         *
         * @see InsertChild()
         */
        void SetParent(Component* _parent);

        /**
         * @brief Prints the whole subtree of this component.
         *        The tree is printed in DFS order, so that the hierarchy can be easily seen.
         *        For each component in the subtree, the following is printed: "<string component type> (name <name>) id <id> - children: <num children>"
         *
         * @pythonBinding
         */
        void PrintSubtree() const;

    private:
        /**
         * @private
         *
         * @brief Helper function for PrintSubtree(), ensures proper indentation.
         *        Using PrintSubtree() is recommended, but this can be used for custom indentation.
         *
         * @param level Number of "  " to print before the component.
         *
         * @see PrintSubtree()
         */
        void _PrintSubtree(int level) const;
    public:
        /**
         * @brief Prints all Relations in the subtree.
         *
         * @param relationType Filter by the given RelationType (default: Any).
         *
         * @pythonBinding
         */
        void PrintRelationsInSubtree(RelationType::type relationType = RelationType::Any);

        /**
         * @brief Returns the name of the component.
         *
         * @return The name.
         *
         * @see name
         */
        const std::string& GetName() const;

        /**
         * @brief Sets the name of the component.
         *
         * @param _name The new name of the component.
         *
         * @see name
         */
        void SetName(const std::string &_name);

        /**
         * @brief Returns the ID of the component.
         *
         * @return The ID.
         *
         * @see id
         */
        int GetId() const;

        /**
         * @brief Sets the ID of the component.
         *
         * @param _id The new ID of the component.
         *
         * @see id
         */
        void SetId(int _id);

        /**
         * @brief Returns the ComponentType of the component.
         *
         * @return The type of the component.
         *
         * @see componentType
         */
        sys_sage::ComponentType::type GetComponentType() const;

        /**
         * @brief Returns the ComponentType as a human-readable string as defined in sys_sage::ComponentType::names.
         *
         * @return A string representation of the component's type.
         *
         * @see componentType
         *
         * @pythonBinding
         */
        const std::string &GetComponentTypeStr() const;

        /**
         * @brief Returns a reference to a constant vector containing all children of the component (empty vector if no children).
         *
         * @return Reference to an immutable vector of children.
         *
         * @see children
         *
         * @pythonBinding
         */
        const std::vector<Component*>& GetChildren() const;

        /**
         * @private
         *
         * @brief Returns a reference to the non-const children vector (internal use).
         *
         * @return Reference to a mutable vector of children.
         *
         * @see children
         */
        std::vector<Component*>& _GetChildren();

        /**
         * @brief Returns a pointer to the component's parent (`nullptr` if this component is the root).
         *
         * @return The component' parent.
         *
         * @see parent
         */
        Component* GetParent() const;

        /**
         * @brief Retrieve a child with a specific ID.
         *        Identical to GetChildById.
         *        \n Should there be more children with the same ID, the first match will be retrieved (i.e. the one with lower index in the children vector).
         *
         * @param _id The target ID.
         *
         * @return The child with the specified ID or `nullptr` if no such child exists.
         *
         * @see GetChildById
         *
         * @pythonBinding
         */
        Component* GetChild(int _id) const;

        /**
         * @brief Retrieve a child with a specific ID.
         *        \n Should there be more children with the same ID, the first match will be retrieved (i.e. the one with lower index in the children vector).
         *
         * @param _id The target ID.
         *
         * @return The child with the specified ID or `nullptr` if no such child exists.
         *
         * @pythonBinding
         */
        Component* GetChildById(int _id) const;

        /**
         * @brief Retrieve a child of a specific type.
         *        \n Should there be more children with the same type, the first match will be retrieved (i.e. the one with lower index in the children vector).
         *
         * @param _componentType The ComponentType to match.
         *
         * @return Pointer to the first matching child, or nullptr if not found.
         *
         * @pythonBinding
         */
        Component* GetChildByType(ComponentType::type _componentType) const;

        /**
         * @brief Searches for all children matching the given ComponentType.
         *
         * @param _componentType Required ComponentType.
         *
         * @return A new vector of all matching children.
         *
         * @pythonBinding
         */
        std::vector<Component*> FindChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Searches for all children matching the given ComponentType.
         *
         * @param _outArray An output vector to append the matching children to.
         * @param _componentType Required ComponentType.
         */
        void FindChildrenByType(std::vector<Component *> &_outArray, ComponentType::type _componentType) const;

        /**
         * @brief Looks for a component with a matching ID and ComponentType within the subtree spanned by this component.
         *        The subtree is traversed in DFS order.
         *
         * @param _id The target ID.
         * @param _componentType The target type of the component.
         *
         * @return Returns the first occurence that matches these criteria.
         *
         * @pythonBinding
         */
        Component *GetDescendantById(int _id, ComponentType::type _componentType);

        /**
         * @brief Searches for all the descendants in the subtree matching the given ComponentType.
         * 
         * @param outArray An output vector to append the matching descendants to.
         * @param _componentType - Required ComponentType.
         */
        void FindDescendantsByType(std::vector<Component*> &outArray, ComponentType::type _componentType);

        /**
         * @brief Searches for all the descendants in the subtree matching the given ComponentType.
         * 
         * @param _componentType - Required ComponentType.
         *
         * @return A new vector of all matching descendants.
         *
         * @pythonBinding
         */
        std::vector<Component*> FindDescendantsByType(ComponentType::type _componentType);

        /**
         * @brief Counts the number of descendants in the subtree matching the requested ComponentType.
         *
         * @param _componentType The ComponentType to look for.
         *
         * @return Aggregate number of descendants matching the criterion.
         *
         * @pythonBinding
         */
        int CountDescendantsByType(ComponentType::type _componentType) const;

        /**
         * @brief Counts the number of children in the subtree matching the requested ComponentType.
         *
         * @param _componentType The ComponentType to look for.
         *
         * @return Aggregate number of children matching the criterion.
         *
         * @pythonBinding
         */
        int CountChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Moves up the tree until a parent of the given type is found.
         *
         * @param _componentType Desired ComponentType.
         *
         * @return Pointer to the ancestor, or nullptr if not found.
         *
         * @pythonBinding
         */
        Component* GetAncestorByType(ComponentType::type _componentType);

        /**
         * @brief Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
         *
         * @return 0 if the component is a leaf;
         *         \n 1 if children are leaves;
         *         \n 2 if at most children's children are leaves;
         *         \n and so on...
         *
         * @pythonBinding
         */
        int CalcSubtreeDepth() const;//0=empty, 1=1element,...

        /**
         * @brief Retrieves the Nth ancestor, which resides N levels above in the tree.
         *
         * @param n How many levels above this component.
         *
         * @returns nullptrs if the number of levels exceeds the distance to the root of the tree;
         *          \n This component if n = 0;
         *          \n The parent if n = 1;
         *          \n The grandparent if n = 2;
         *          \n and so on...
         *
         * @pythonBinding
         */
        Component* GetNthAncestor(int n);

        /**
         * @brief Retrieves all descendants N levels below this component relative to the subtree spanned by this component.
         *        The subtree is traversed in DFS order.
         *
         *        \n If N is larger than the subtree depth, then no descandants will be retrieved;
         *        \n If N = 1, then the children will be retrievd;
         *        \n If N = 2, then the grandchildren will be retrieved;
         *        \n and so on...
         * 
         * @param outArray An output vector to append the matching descendants to.
         * @param depth How many levels below this component.
         */
        void FindNthDescendants(std::vector<Component*> &outArray, int depth);

        /**
         * @brief Retrieves all descendants N levels below this component relative to the subtree spanned by this component.
         *        The subtree is traversed in DFS order.
         *
         * @param depth How many levels below this component.
         *
         * @return A new vector of all matching descendants.
         *
         *        \n If N is larger than the subtree depth, then no descandants will be retrieved;
         *        \n If N = 1, then the children will be retrievd;
         *        \n If N = 2, then the grandchildren will be retrieved;
         *        \n and so on...
         *
         * @pythonBinding
         */
        std::vector<Component*> FindNthDescendants(int depth);

        /**
         * @brief Returns a reference to a constant vector containing all relations of the component that are of a specific type.
         *
         * @param relationType Type of relation (can be RelationType::Any).
         *
         * @return Reference to an immutable vector of relations (may be empty).
         *
         * @see relations
         *
         * @pythonBinding
         */
        const std::vector<Relation*>& GetRelationsByType(RelationType::type relationType) const;

        /**
         * @private
         *
         * @brief Returns a reference to a non-const vector containing all relations of the component that are of a specific type (internal use).
         *
         * @param relationType Type of relation (can be RelationType::Any).
         *
         * @return Reference to a mutable vector of relations.
         *
         * @see relations
         */
        std::vector<Relation*>& _GetRelationsByType(RelationType::type relationType);

        /**
         * @brief Returns a newly-constructed vector of all relations of a given type and where this component is at a specified position.
         *
         * @param relationType Type of relation (default: Any).
         * @param thisComponentPosition Position of this component relative to the sys_sage::Relation::components vector of the relation (default: -1 = do NOT care about position).
         *
         * @return A new vector of all matching relations.
         *
         * @pythonBinding
         */
        std::vector<Relation*> FindRelations(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) const;

        /**
         * @private
         *
         * @brief Adds the given relation to the internal vector of this component's relations.
         *        Only called by Relation's AddComponent/UpdateComponent (internal use).
         *
         * @param relationType Type of relation.
         *
         * @param r Pointer to the relation.
         */
        void _AddRelation(RelationType::type relationType, Relation* r);

        /**
         * @brief Retrieves a DataPath from the vector of this component's data paths with matching DataPathCategory and DataPathDirection.
         *        The first match is returned.
         *
         * @param dp_category DataPath category to search for.
         * @param direction Orientation of the target DataPath (default: Any).
         *
         * @return Pointer to the found DataPath, or nullptr if not found.
         *
         * @pythonBinding
         */
        DataPath* GetDataPathByCategory(DataPathCategory::type dp_category, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Searches for all data paths of this component matching the given DataPathCategory and DataPathDirection.
         *
         * @param outDpArr An output vector to append the matching data paths to.
         * @param dp_category Required DataPathCategory (default: Any).
         * @param direction Required Direction/Orientation of a DataPath (default: Any).
         */
        void FindDataPaths(std::vector<DataPath*> &outDpArr, DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Searches for all data paths of this component matching the given DataPathCategory and DataPathDirection.
         *
         * @param dp_category Required DataPathCategory (default: Any).
         * @param direction Required Direction/Orientation of a DataPath (default: Any).
         *
         * @return A new vector of all matching data paths.
         *
         * @pythonBinding
         */
        std::vector<DataPath*> FindDataPaths(DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Checks the consistency of the subtree spanned by this component.
         *        This function verifies that each child component has this component set as its parent.
         *        It logs an error message for each child that has an incorrect parent and increments the error count.
         *        The function then recursively checks the consistency of the entire subtree rooted at each child component.
         *
         * @return The total number of inconsistencies found in the subtree.
         *
         * @pythonBinding
         */
        int CheckSubtreeConsistency() const;

        /**
         * @brief Calculates approximate memory footprint of the subtree spanned by this component (including the relevant relations).
         *
         * @param out_component_size Output parameter storing the footprint of the components in the subtree.
         *                           It is assumed to be allocated from the user-side and initialized with the value 0 (the result is accumulated here).
         * @param out_RelationSize Output parameter storing the footprint of the relations in the subtree.
         *                         It is assumed to be allocated from the user side and initialized with the value 0 (the result is accumulated here).
         *
         * @return The total size in bytes.
         */
        int CalcSubtreeSize(unsigned *out_component_size, unsigned *out_RelationSize) const;

        /**
         * @private
         *
         * @brief Calculates approximate memory footprint of the subtree spanned by this component (including the relevant relations).
         *        Used as a helper function for internal use.
         *
         * @param out_component_size Output parameter storing the footprint of the components in the subtree.
         *                           It is assumed to be allocated from the user-side and initialized with the value 0 (the result is accumulated here).
         * @param out_RelationSize Output parameter storing the footprint of the relations in the subtree.
         *                         It is assumed to be allocated from the user side and initialized with the value 0 (the result is accumulated here).
         * @param countedRelations Set of already traversed relations.
         *
         * @return The total size in bytes.
         */
        int _CalcSubtreeSize(unsigned *out_component_size, unsigned *out_RelationSize, std::set<Relation*> &countedRelations) const;

        /**
         * @brief Retrieves the depth (level) of a component in the ComponentTree.
         *
         * @param refresh If true, recalculate the position (depth) of the component in the tree;
         *                \n if false, return the already stored value.
         *
         * @return The depth (level) of the component.
         *
         * @see depth
         *
         * @pythonBinding
         */
        int CalcDepth(bool refresh);

        // TODO: remove in the future
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
        virtual void _ToJson(nlohmann::json &obj) const;

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
        virtual int _FromJson(const nlohmann::json &obj);

        /**
         * @brief Deletes all relations of this component (optionally filtered by type).
         *        This assumes that all the relations are HEAP-ALLOCATED.
         *
         * @param relationType Relation type to delete (default: Any).
         *
         * @pythonBinding
         */
        void DeleteRelations(RelationType::type relationType = RelationType::Any);

#ifdef SS_PAPI
        /**
         * @brief Prints all PAPI metrics of the given event set within the
         *        subtree spanned by this component.
         *
         * @param eventSet Specifies the event set of interest. If the value is
         *        `PAPI_NULL`, then all event sets will be printed.
         *
         * @pythonBinding
         */
        void PrintPAPImetricsInSubtree(int eventSet = -1) const;

        /**
         * @brief Retrieves all relations used for collecting PAPI metrics
         *        within the subtree spanned by this component.
         *
         * @return A vector containing said relations.
         *
         * @pythonBinding
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
         *
         * @pythonBinding
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
         *
         * @pythonBinding
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
         *
         * @pythonBinding
         */
        attribSizeType GetAttributesSize() const;

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
         *
         * @pythonBinding
         */
        void EraseAttribute(const std::string &key);

        /**
         * @brief Removes the attribute of the given iterator.
         *
         * @param it The iterator of the attribute.
         *
         * @return The iterator to the next attribute.
         */
        attribIterator EraseAttribute(attribIterator it);

        /**
         * @brief Removes all attributes.
         *
         * @pythonBinding
         */
        void ClearAttributes();

    protected:
        /**
         * @brief Protected constructor for derived classes (no automatic insertion in the ComponentTree).
         *
         * @param _id Numeric ID of the component.
         * @param _name Name of the component.
         * @param _componentType ComponentType of the component.
         */
        Component(int _id, const std::string &_name, ComponentType::type _componentType);

        /**
         * @brief Protected constructor for derived classes with insertion into the ComponentTree.
         *
         * @param _parent Pointer to the parent component.
         * @param _id Numeric ID of the component.
         * @param _name Name of the component.
         * @param _componentType ComponentType of the component.
         */
        Component(Component *_parent, int _id, const std::string &_name, ComponentType::type _componentType);

        int id; /**< Numeric ID of the component. There is no requirement for uniqueness of the ID, however it is advised to have unique IDs at least in the realm of parent's children (siblings). Some tree search functions, which take the ID as a parameter, search for first matching ID, meaning that the user is responsible to manage uniqueness in the realm of the search subtree (or should be aware of the consequences of not doing so). */
        int depth; /**< Depth (level) of the Component in the ComponentTree. */
        std::string name; /**< Name of the component. */
        int count{-1}; /**< Can be used to represent multiple components with the same properties. By default, it represents only 1 component, and is set to -1. */ // TODO: maybe set the default to 1?
        const ComponentType::type componentType; /**< Indicates the type of a component (see sys_sage::ComponentType). Can be used to cast a given base `Component *` pointer into one of the derived classes. */
        std::vector<Component*> children; /**< Contains the vector of pointers to children of the component in the ComponentTree. */
        Component* parent { nullptr }; /**< Contains a pointer to the parent component in the ComponentTree. If this component is the root, the parent will be nullptr. */
        std::array<std::vector<Relation*>*, RelationType::_num_relation_types>* relations = nullptr; /**< Collection of all relations of this component. It is initialized once the first relation is added. Each entry of the array corresponds to one of the RelationTypes (see sys_sage::RelationType) and consists of a (lazy-allocated) vector of relations of that type. */
        std::map<std::string, std::unique_ptr<IAttribute>> attributes; /**< Stores all attributes of this component. */
    };

} //namespace sys_sage 

#include <sys-sage/Component.inl>

#endif //COMPONENT_HPP

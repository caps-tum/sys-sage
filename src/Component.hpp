#ifndef COMPONENT
#define COMPONENT

#include <array>
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "defines.hpp"
#include "enums.hpp"
#include "DataPath.hpp"
#include <libxml/parser.h>



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
        Component(int _id = 0, std::string _name = "unknown");
        /**
         * @brief Generic Component constructor with insertion into the Component Tree as the parent's child.
         * Usually one of the derived subclasses for different Component Types will be created.
         * @param parent Pointer to the parent component
         * @param _id Numeric ID of the component (default 0)
         * @param _name Name of the component (default "unknown")
         *
         * Sets componentType to sys_sage::ComponentType::Generic.
         */
        Component(Component * parent, int _id = 0, std::string _name = "unknown");
        //SVTODO reevaluate the delete vs destructor
        /**
         * @private
         * @brief Use Delete() or DeleteSubtree() for deleting and deallocating the components.
         */
        virtual ~Component() = default;
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
         * @brief Removes the passed component from the list of children, without completely deleting (and deallocating) the child itself
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
        /**
         * @deprecated Use PrintRelationsInSubtree instead. This function will be removed in the future.
         * @brief Prints all DataPaths that go from or to components in the subtree.
         * @see PrintAllRelationsInSubtree( RelationType::type RelationType = RelationType::Any)
         * @note This function is deprecated and will be removed in the future. Use PrintRelationsInSubtree instead.
         */
    public:
        [[ deprecated("Use PrintRelationsInSubtree instead. This function will be removed in the future (used up until version 0.5.2).") ]]
        void PrintAllDataPathsInSubtree();

        /**
         * @brief Prints all Relations in the subtree.
         * @param RelationType Filter by relation type (default: Any)
         */
        [[ deprecated("Use PrintRelationsInSubtree instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void PrintAllRelationsInSubtree(RelationType::type RelationType = RelationType::Any);

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
        void SetName(std::string _name);
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
        std::string GetComponentTypeStr() const;
        /**
         * @brief Returns a const reference to std::vector containing all children of the component (empty vector if no children).
         * @return const std::vector<Component *> & with children
         */
        const std::vector<Component*>& GetChildren() const;
        /**
         * @private
         * @brief Returns a non-const reference to the children vector (internal use).
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
        [[ deprecated("Use FindChildrenByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetAllChildrenByType(ComponentType::type _componentType) const;

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
        [[ deprecated("Use FindChildrenByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetAllChildrenByType(std::vector<Component *> *_outArray, ComponentType::type _componentType) const;

        /**
         * @brief Searches for all the children matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void FindChildrenByType(std::vector<Component *> *_outArray, ComponentType::type _componentType) const;

        /**
        * @brief Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
        * @return Returns first occurence that matches these criteria.
        * @param _id - the id to look for
        * @param _componentType - the component type where to look for the id
        * @return Component * matching the criteria. Returns the first match. NULL if no match found
        */
        [[ deprecated("Use GetDescendantById instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        Component* GetSubcomponentById(int _id, ComponentType::type _componentType);

        /**
        * @brief Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
        * @return Returns first occurence that matches these criteria.
        * @param _id - the id to look for
        * @param _componentType - the component type where to look for the id
        * @return Component * matching the criteria. Returns the first match. NULL if no match found
        */
        Component *GetDescendantById(int _id, ComponentType::type _componentType);

        /**
         * @brief Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetAllSubcomponentsByType(std::vector<Component*>* outArray, ComponentType::type _componentType);

        /**
         * @brief Searches for all the descendants matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void FindDescendantsByType(std::vector<Component*>* outArray, ComponentType::type _componentType);

        /**
         * @brief Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components.
         * @returns A vector of all the subcomponents matching the _componentType.
        */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetAllSubcomponentsByType(ComponentType::type _componentType);

        /**
         * @brief Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components.
         * @returns A vector of all the subcomponents matching the _componentType.
        */
        std::vector<Component*> FindDescendantsByType(ComponentType::type _componentType);

        /**
         * @brief Counts number of subcomponents (children, grandchildren, etc.).
         * @return Number of subcomponents
         */
        [[ deprecated("Use CountDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CountAllSubcomponents() const;
        
        /**
         * @brief Counts number of subcomponents (children, their children and so on) matching the requested component type.
         * @param _componentType - ComponentType to look for.
         * @return Returns number of subcomponents matching the requested component type.
         */
        [[ deprecated("Use CountDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CountAllSubcomponentsByType(ComponentType::type _componentType) const;

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
        [[ deprecated("Use CountChildrenByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CountAllChildrenByType(ComponentType::type _componentType) const;

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
        [[ deprecated("Use CalcSubtreeDepth instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int GetSubtreeDepth() const;//0=empty, 1=1element,...

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
        [[ deprecated("Use FindNthDescendants instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetNthDescendents(std::vector<Component*>* outArray, int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
         */
        void FindNthDescendants(std::vector<Component*>* outArray, int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. 
         * The tree is traversed in order as the children are stored in the std::vector.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @return A std::vector<Component*> with the results.
         */
        [[ deprecated("Use FindNthDescendants instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetNthDescendents(int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. 
         * The tree is traversed in order as the children are stored in the std::vector.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @return A std::vector<Component*> with the results.
         */
        std::vector<Component*> FindNthDescendants(int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. 
         * The tree is traversed DFS in order as the children are stored in each std::vector children.
         * @param componentType - componentType
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetSubcomponentsByType(std::vector<Component*>* outArray, ComponentType::type componentType);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. 
         * The tree is traversed DFS in order as the children are stored in the std::vector.
         * @param componentType - componentType
         * @return A std::vector<Component*> with the results.
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetSubcomponentsByType(ComponentType::type _componentType);

        /**
         * @brief Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetComponentsInSubtree(std::vector<Component*>* outArray);

        /**
         * @brief Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
         * @return A std::vector<Component*> with the results.
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetComponentsInSubtree();

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/DeleteRelation() to modify the list of Relations, or access the Relations' API directly.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        [[ deprecated("Use GetRelationsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        const std::vector<Relation*>& GetRelations(RelationType::type relationType) const;

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/DeleteRelation() to modify the list of Relations, or access the Relations' API directly.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        const std::vector<Relation*>& GetRelationsByType(RelationType::type relationType) const;

        /**
         * @private
         * @brief Returns a non-const reference to the internal vector of relations (should not be called externally -- you can break things).
         * @param relationType Type of relation
         * @return std::vector<Relation*>& (reference to internal structure)
         * @see public alternative GetRelations(RelationType::type relationType) const
         */
        [[ deprecated("Use _GetRelationsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Relation*>& _GetRelations(RelationType::type relationType) const;

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/DeleteRelation() to modify the list of Relations, or access the Relations' API directly.
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
        [[ deprecated("Use FindRelations instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Relation*> GetAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) const;

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
         * @brief Retrieves a DataPath* from the list of this component's data paths with matching DataPathType and DataPathDirection.
         * The first match is returned.
         * @param dp_type DataPath type to search for
         * @param direction Orientation (default: Any)
         * @return Pointer to the found DataPath, or nullptr if not found
         */
        DataPath* GetDataPathByType(DataPathType::type dp_type, DataPathDirection::type direction = DataPathDirection::Any) const;
        
        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation.
         * Results are returned in std::vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
         * @param outDpArr - output parameter (vector with results)
         * An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
         * The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         * @param dp_type DataPath type to search for (default: Any)
         * @param direction Orientation/direction of a DataPath (default: Any)
         */
        [[ deprecated("Use FindDataPaths instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetAllDataPaths(std::vector<DataPath*>* outDpArr, DataPathType::type dp_type = DataPathType::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation.
         * Results are returned in std::vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
         * @param outDpArr - output parameter (vector with results)
         * An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
         * The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         * @param dp_type DataPath type to search for (default: Any)
         * @param direction Orientation/direction of a DataPath (default: Any)
         */
        void FindDataPaths(std::vector<DataPath*>* outDpArr, DataPathType::type dp_type = DataPathType::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation/direction.
         * Results are returned in a std::vector<DataPath*>*.
         * @param dp_type DataPath type to search for (default: Any)
         * @param direction Orientation (default: Any)
         * @return Vector of matching DataPaths
         */
        [[ deprecated("Use FindDataPaths instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<DataPath*> GetAllDataPaths(DataPathType::type dp_type = DataPathType::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation/direction.
         * Results are returned in a std::vector<DataPath*>*.
         * @param dp_type DataPath type to search for (default: Any)
         * @param direction Orientation (default: Any)
         * @return Vector of matching DataPaths
         */
        std::vector<DataPath*> FindDataPaths(DataPathType::type dp_type = DataPathType::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
        @brief Checks the consistency of the component tree starting from this component.

        This function verifies that each child component has this component set as its parent.
        It logs an error message for each child that has an incorrect parent and increments the error count.
        The function then recursively checks the consistency of the entire subtree rooted at each child component.

        @return The total number of inconsistencies found in the component tree.
        
        The function returns the total number of errors found in the component tree, including errors in the direct children and any nested descendants.
        */
        [[ deprecated("Use CheckSubtreeConsistency instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CheckComponentTreeConsistency() const;

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
        [[ deprecated("Use CalcSubtreeSize instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize) const;

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
        [[ deprecated("Use _CalcSubtreeSize instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int _GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const;

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
        int _CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const;

        /**
         * @brief Retrieves the depth (level) of a component in the topology.
         * @param refresh If true, recalculate the position (depth) of the component in the tree; if false, return the already stored value
         * @return Depth (level) of the component in the topology
         * @see depth
         */
        [[ deprecated("Use CalcDepth instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int GetDepth(bool refresh);

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
         * @brief Deletes a Relation from this component as well as the Relation itself.
         * @param r Pointer to the relation to delete
         * @see Relation/DataPath/QuantumGate Delete()
         */
        void DeleteRelation(Relation * r);
        /**
         * @deprecated Use void DeleteRelation(Relation * r) instead.
         * @brief Deletes and deallocates the DataPath pointer from the list of outgoing/incoming DataPaths.
         * @param dp DataPath to delete
         */
        [[deprecated("DeleteDataPath is deprecated. Use void DeleteRelation(Relation * r) instead (used up until version 0.5.2).")]]
        void DeleteDataPath(DataPath * dp);

        /**
         * @brief Deletes all relations of this component (optionally filtered by type).
         * @param relationType Relation type to delete (default: Any)
         */
        [[ deprecated("Use DeleteRelations instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void DeleteAllRelations(RelationType::type relationType = RelationType::Any);

        /**
         * @brief Deletes all relations of this component (optionally filtered by type).
         * @param relationType Relation type to delete (default: Any)
         */
        void DeleteRelations(RelationType::type relationType = RelationType::Any);

        /**
         * @deprecated Use void DeleteRelations(RelationType::type relationType = sys_sage::RelationType::Any) instead.
         * @brief Deletes all DataPaths of this component.
         */
        [[ deprecated("Use DeleteRelations instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void DeleteAllDataPaths();
        /**
        Deletes the whole subtree (all the children) of the component.
        */
        void DeleteSubtree() const;
        /**
         * @brief Deletes a component, its children (if withSubtree = true), and all associated Relations.
         * If only the component itself is deleted, its children are inserted into its parent's children list.
         * @param withSubtree If true, the whole subtree is deleted; otherwise only the component itself.
         */
        void Delete(bool withSubtree = true);

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
        
    protected:
        /**
         * @brief Protected constructor for derived classes (no automatic insertion in the Component Tree).
         * @param _id Numeric ID of the component
         * @param _name Name of the component
         * @param _componentType Component type (of type sys_sage::ComponentType::type)
         */
        Component(int _id, std::string _name, ComponentType::type _componentType);

        /**
         * @brief Protected constructor for derived classes with insertion into the Component Tree.
         * @param parent Pointer to the parent component
         * @param _id Numeric ID of the component
         * @param _name Name of the component
         * @param _componentType Component type (of type sys_sage::ComponentType::type)
         */
        Component(Component * parent, int _id, std::string _name, ComponentType::type _componentType);

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
    };

} //namespace sys_sage 
#endif //COMPONENT_HPP

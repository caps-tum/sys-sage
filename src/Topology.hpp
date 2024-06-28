#ifndef TOPOLOGY
#define TOPOLOGY

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "defines.hpp"
#include "DataPath.hpp"
#include <libxml/parser.h>


#define SYS_SAGE_COMPONENT_NONE 1 /**< class Component (do not use normally)*/
#define SYS_SAGE_COMPONENT_THREAD 2 /**< class Thread */
#define SYS_SAGE_COMPONENT_CORE 4 /**< class Core */
#define SYS_SAGE_COMPONENT_CACHE 8 /**< class Cache */
#define SYS_SAGE_COMPONENT_SUBDIVISION 16 /**< class Subdivision */
#define SYS_SAGE_COMPONENT_NUMA 32 /**< class Numa */
#define SYS_SAGE_COMPONENT_CHIP 64 /**< class Chip */
#define SYS_SAGE_COMPONENT_MEMORY 128 /**< class Memory */
#define SYS_SAGE_COMPONENT_STORAGE 256 /**< class Storage */
#define SYS_SAGE_COMPONENT_NODE 512 /**< class Node */
#define SYS_SAGE_COMPONENT_TOPOLOGY 1024 /**< class Topology */

#define SYS_SAGE_SUBDIVISION_TYPE_NONE 1 /**< Generic Subdivision type. */
#define SYS_SAGE_SUBDIVISION_TYPE_GPU_SM 2 /**< Subdivision type for GPU SMs */

#define SYS_SAGE_CHIP_TYPE_NONE 1 /**< Generic Chip type. */
#define SYS_SAGE_CHIP_TYPE_CPU 2 /**< Chip type used for a CPU. */
#define SYS_SAGE_CHIP_TYPE_CPU_SOCKET 4 /**< Chip type used for one CPU socket. */
#define SYS_SAGE_CHIP_TYPE_GPU 8 /**< Chip type used for a GPU.*/


using namespace std;
class DataPath;

/**
Generic class Component - all components inherit from this class, i.e. this class defines attributes and methods common to all components.
\n Therefore, these can be used universally among all components. Usually, a Component instance would be an instance of one of the child classes, but a generic component (instance of class Component) is also possible.
*/
class Component {
public:
    /**
    Generic Component constructor (no automatic insertion in the Component Tree). Usually one of the derived subclasses for different Component Types will be created. Sets:
    @param _id = id, default 0
    @param _name = name, default "unknown"
    @param _componentType = componentType, default SYS_SAGE_COMPONENT_NONE
    */
    Component(int _id = 0, string _name = "unknown", int _componentType = SYS_SAGE_COMPONENT_NONE);
    /**
    Generic Component constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Usually one of the derived subclasses for different Component Types will be created. Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "unknown"
    @param _componentType = componentType, default SYS_SAGE_COMPONENT_NONE
    */
    Component(Component * parent, int _id = 0, string _name = "unknown", int _componentType = SYS_SAGE_COMPONENT_NONE);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    virtual ~Component() = default;
    /**
    Inserts a Child component to this component (in the Component Tree).
    The child pointer will be inserted at the end of std::vector of children (retrievable through GetChildren(), GetChild(int _id) etc.)
    @param child - a pointer to a Component (or any class instance that inherits from Component).
    @see GetChildren()
    @see GetChild(int _id)
    */
    void InsertChild(Component * child);
    
    /**
     * Inserts this component between a parent and one of its children. The parent component remains the parent, this Component becomes a new child of the parent, and the specified child becomes this component's child.
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
     * Inserts this component between a parent and a (subset of) his children. The parent component remains parent, this Component becomes a new child, and the children become parent's grandchildren.
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
    int InsertBetweenParentAndChildren(Component* parent, vector<Component*> children, bool alreadyParentsChild);
    
    /**
    Removes the passed component from the list of children, without completely deleting (and deallocating) the child itself
    @param child - child to remove
    @return how many elements were deleted (normally, 0 or 1 should be possible)
    */
    int RemoveChild(Component * child);
    /**
    Define a parent to the component. This is usually used when inserting a component in the tree (by calling InsertChild on the parent, and calling SetParent on the child).
    @param parent - a pointer to a Component (or any class instance that inherits from Component).
    @see InsertChild()
    */
    void SetParent(Component* parent);
    /**
    Prints the whole subtree of this component (including the component itself) to stdout. The tree is printed in DFS order, so that the hierarchy can be easily seen. Each child is indented by "  ".
    For each component in the subtree, the following is printed: "<string component type> (name <name>) id <id> - children: <num children>
    */
    void PrintSubtree();

    /**
    @private
    Helper function of PrintSubtree();, which ensures the proper indentation. Using PrintSubtree(); is recommended, however this method can be used as well.
    @param level - number of "  " to print out in front of the first component.
    @see PrintSubtree();
    */
    void PrintSubtree(int level);
    /**
    Prints to stdout basic information about all DataPaths that go either from or to the components in the subtree.
    \n For each component, all outgoing and incoming DataPaths are printed, i.e. a DataPath may be printed twice.
    */
    void PrintAllDataPathsInSubtree();
    /**
    Returns name of the component.
    @return name
    @see name
    */
    string GetName();
    /**
    Sets name of the component.
    @param _name - name of the component
    @see name
    */
    void SetName(string _name);
    /**
    Returns id of the component.
    @return id
    @see id
    */
    int GetId();
    /**
    Returns component type of the component. The component type denotes of which class the instance is (Often the components are stored as Component*, even though they are a member of one of the child classes)
    \n SYS_SAGE_COMPONENT_NONE -> class Component
    \n SYS_SAGE_COMPONENT_THREAD -> class Thread
    \n SYS_SAGE_COMPONENT_CORE -> class Core
    \n SYS_SAGE_COMPONENT_CACHE -> class Cache
    \n SYS_SAGE_COMPONENT_SUBDIVISION -> class Subdivision
    \n SYS_SAGE_COMPONENT_NUMA -> class Numa
    \n SYS_SAGE_COMPONENT_CHIP -> class Chip
    \n SYS_SAGE_COMPONENT_MEMORY -> class Memory
    \n SYS_SAGE_COMPONENT_STORAGE -> class Storage
    \n SYS_SAGE_COMPONENT_NODE -> class Node
    \n SYS_SAGE_COMPONENT_TOPOLOGY -> class Topology
    @returns componentType
    @see componentType
    */
    int GetComponentType();
    /**
    Returns component type in human-readable string.
    \n SYS_SAGE_COMPONENT_NONE -> "None"
    \n SYS_SAGE_COMPONENT_THREAD -> "HW_thread"
    \n SYS_SAGE_COMPONENT_CORE -> "Core"
    \n SYS_SAGE_COMPONENT_CACHE -> "Cache"
    \n SYS_SAGE_COMPONENT_SUBDIVISION -> "Subdivision"
    \n SYS_SAGE_COMPONENT_NUMA -> "NUMA"
    \n SYS_SAGE_COMPONENT_CHIP -> "Chip"
    \n SYS_SAGE_COMPONENT_MEMORY -> "Memory"
    \n SYS_SAGE_COMPONENT_STORAGE -> "Storage"
    \n SYS_SAGE_COMPONENT_NODE -> "Node"
    \n SYS_SAGE_COMPONENT_TOPOLOGY -> "Topology"
    @returns string component type
    @see componentType
    */
    string GetComponentTypeStr();
    /**
    Returns a pointer to std::vector containing all children of the component (empty vector if no children) .
    @returns vector<Component *> * with children
    */
    vector<Component*>* GetChildren();
    /**
    Returns Component pointer to parent (or NULL if this component is the root)
    */
    Component* GetParent();
    /**
    Identical to GetChildById
    Retrieve a Component* to a child with child.id=_id.
    \n Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
    @see GetChildById
    */
    Component* GetChild(int _id);

    /**
    Retrieve a Component* to a child with child.id=_id.
    \n Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
    */
    Component* GetChildById(int _id);

    /**
    Retrieve a Component* to a child matching the given component type.
    \n Should there be more children with the same type, the first match will be retrieved (i.e. the one with lower index in the children array.)
    */
    Component* GetChildByType(int _componentType);

    /**
     * Searches for all the children matching the given component type.
     * 
     * @param _componentType - Required type of components
     * @returns A vector of all the children matching the _componentType
    */
    vector<Component*> GetAllChildrenByType(int _componentType);

    /**
     * Searches for all the children matching the given component type.
     * 
     * @param _componentType - Required type of components
     * @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
    */
    void GetAllChildrenByType(vector <Component *> *_outArray, int _componentType);
    /**
    OBSOLETE. Use GetSubcomponentById instead. This function will be removed in the future.
    */
    [[deprecated("Use GetSubcomponentById instead. This function will be removed in the future.")]]
    Component* FindSubcomponentById(int _id, int _componentType);
    /**
    @private
    OBSOLETE. Use GetAllSubcomponentsByType instead. This function will be removed in the future.
    */
   [[deprecated("Use GetAllSubcomponentsByType instead. This function will be removed in the future.")]]
    void FindAllSubcomponentsByType(vector<Component*>* outArray, int _componentType);
    /**
    Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
    \n Returns first occurence that matches these criteria.
    @param _id - the id to look for
    @param _componentType - the component type where to look for the id
    @return Component * matching the criteria. Returns the first match. NULL if no match found
    */
    Component* GetSubcomponentById(int _id, int _componentType);
    
    /**
     * Searches for all the subcomponents (children, their children and so on) matching the given component type.
     * 
     * @param _componentType - Required type of components
     * @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
    */
    void GetAllSubcomponentsByType(vector<Component*>* outArray, int _componentType);
    
    /**
     * Searches for all the subcomponents (children, their children and so on) matching the given component type.
     * 
     * @param _componentType - Required type of components.
     * @returns A vector of all the subcomponents matching the _componentType.
    */
    vector<Component*> GetAllSubcomponentsByType(int _componentType);
    
    /**
    Counts number of subcomponents (children, their children and so on).
    @return Returns number of subcomponents.
    */
    int CountAllSubcomponents();
    
    /**
    Counts number of subcomponents (children, their children and so on) matching the requested component type.
    @param _componentType - Component type to look for.
    @return Returns number of subcomponents matching the requested component type.
    */
    int CountAllSubcomponentsByType(int _componentType);

    /**
    Counts number of children matching the requested component type.

    @param _componentType - Component type to look for.
    @return Returns number of children matching the requested component type.
    */
    int CountAllChildrenByType(int _componentType);
    
    /**
    Moves up the tree until a parent of given type.
    @param _componentType - the desired component type
    @return Component * matching the criteria. NULL if no match found
    */
    Component* GetAncestorByType(int _componentType);
    /**
    @private 
    OBSOLETE. Use GetAncestorByType instead. This function will be removed in the future.
    */
   [[deprecated("Use GetAncestorByType instead. This function will be removed in the future.")]]
    Component* FindParentByType(int _componentType);

    /**
    OBSOLETE. Use int CountAllSubcomponentsByType(SYS_SAGE_COMPONENT_THREAD) instead.
    Returns the number of Components of type SYS_SAGE_COMPONENT_THREAD in the subtree.
    */
   [[deprecated("Use int CountAllSubcomponentsByType(SYS_SAGE_COMPONENT_THREAD) instead.")]]
    int GetNumThreads();
    
    /**
    Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
    \n 0=leaf, 1=children are leaves, 2=at most children's children are leaves .....
    @return maximal distance to a leaf
    */
    int GetSubtreeDepth();//0=empty, 1=1element,...
    
    /**
    Retrieves Nth ancestor, which resides N levels above. 
    \n E.g. if n=1, the parent is retrieved; if n=2, the grandparent is retrieved and so on.
    @param n - how many levels above the tree should be looked.
    @returns The ancestor residing N levels above.
    */
    Component* GetNthAncestor(int n);

    /**
    Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
    \n E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
    @param depth - how many levels down the tree should be looked
    @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
    */
    void GetNthDescendents(vector<Component*>* outArray, int depth);

    /**
    Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in the std::vector.
    \n E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
    @param depth - how many levels down the tree should be looked
    @return A std::vector<Component*> with the results.
    */
    vector<Component*> GetNthDescendents(int depth);

    /**
    Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. The tree is traversed DFS in order as the children are stored in each std::vector children.
    @param componentType - componentType
    @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
    */
    void GetSubcomponentsByType(vector<Component*>* outArray, int componentType);

    /**
    Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. The tree is traversed DFS in order as the children are stored in the std::vector.
    @param componentType - componentType
    @return A std::vector<Component*> with the results.
    */
    vector<Component*> GetSubcomponentsByType(int _componentType);
    
    /** 
    Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
    @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
    */
    void GetComponentsInSubtree(vector<Component*>* outArray);

    /**  
    Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
    @return A std::vector<Component*> with the results.
    */
    vector<Component*> GetComponentsInSubtree();

    /**
    Returns the DataPaths of this component according to their orientation.
    @param orientation - either SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING
    @return Pointer to std::vector<DataPath *> with the result (dp_outgoing on SYS_SAGE_DATAPATH_OUTGOING, or dp_incoming on SYS_SAGE_DATAPATH_INCOMING, otherwise NULL)
    @see dp_incoming
    @see dp_outgoing
    */
    vector<DataPath*>* GetDataPaths(int orientation);
    /**
    @private
    !!Normally should not be called; Use NewDataPath() instead!!
    Stores (pushes back) a DataPath pointer to the list(std::vector) of DataPaths of this component. According to the orientation param, the proper list is chosen.
    @param p - the pointer to store
    @param orientation - orientation of the DataPath. Either SYS_SAGE_DATAPATH_OUTGOING (lands in dp_outgoing) or SYS_SAGE_DATAPATH_INCOMING (lands in dp_incoming)
    @see NewDataPath()
    */
    void AddDataPath(DataPath* p, int orientation);
    /**
    Retrieves a DataPath * from the list of this component's data paths with matching type and orientation.
    \n The first match is returned -- first SYS_SAGE_DATAPATH_OUTGOING are searched, then SYS_SAGE_DATAPATH_INCOMING.
    @param dp_type - DataPath type (dp_type) to search for
    @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
    @return DataPath pointer to the found data path; NULL if nothing found.
    */
    DataPath* GetDataPathByType(int dp_type, int orientation);
    /**
    Retrieves all DataPath * from the list of this component's data paths with matching type and orientation.
    Results are returned in vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
    @param dp_type - DataPath type (dp_type) to search for.
    @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
    @param outDpArr - output parameter (vector with results)
        \n An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
    */
    void GetAllDataPathsByType(vector<DataPath*>* outDpArr, int dp_type, int orientation);

    /**
    Retrieves all DataPath * from the list of this component's data paths with matching type and orientation.
    Results are returned in a vector<DataPath*>*, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
    @param dp_type - DataPath type (dp_type) to search for.
    @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
    @return A std::vector<DataPath*> with the results.
    */
    vector<DataPath*> GetAllDataPathsByType(int dp_type, int orientation);

    /**
    @brief Checks the consistency of the component tree starting from this component.

    This function verifies that each child component has this component set as its parent.
    It logs an error message for each child that has an incorrect parent and increments the error count.
    The function then recursively checks the consistency of the entire subtree rooted at each child component.

    @return The total number of inconsistencies found in the component tree.
    
    The function returns the total number of errors found in the component tree, including errors in the direct children and any nested descendants.
    */
    int CheckComponentTreeConsistency();
    /**
    Calculates approximate memory footprint of the subtree of this element (including the relevant data paths).
    @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
    @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
    @return The total size in bytes
    */
    int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
    /**
    @private
    Helper function of int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize); -- normally you would call this one.
    \n Calculates approximate memory footprint of the subtree of this element (including the relevant data paths). Does not count DataPaths stored in counted_dataPaths.
    @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
    @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
    @param counted_dataPaths - std::set<DataPath*>* of data paths that should not be counted
    @return The total size in bytes
    @see GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
    */
    int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize, std::set<DataPath*>* counted_dataPaths);

    /**
    @private
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
    
    /**
    Deletes and de-allocates the DataPath pointer from the list(std::vector) of outgoing and incoming DataPaths of the Components.
    @param dp - DataPath to Delete
    */
    void DeleteDataPath(DataPath * dp);

    /**
    Deletes all DataPaths of this component.
    */
    void DeleteAllDataPaths();
    /**
    Deletes the whole subtree (all the children) of the component.
    */
    void DeleteSubtree();
    /**
    Deletes a component, its children (if withSubtree = true) and all the associated data paths.
    If only the component itself is deleted, its children are inserted into its parent's children list.
    @param withSubtree - if true, the whole subtree is deleted, otherwise only the component itself.
    */
    void Delete(bool withSubtree = true);

    /**
    For storing arbitrary pieces of information or data. key denotes the name of the attribute,
    and the value points to the data.
    */
    map<string,void*> attrib;
protected:

    int id; /**< Numeric ID of the component. There is no requirement for uniqueness of the ID, however it is advised to have unique IDs at least in the realm of parent's children. Some tree search functions, which take the id as a search parameter search for first match, so the user is responsible to manage uniqueness in the realm of the search subtree (or should be aware of the consequences of not doing so). Component's ID is set by the constructor, and is retrieved via int GetId(); */
    int depth; /**< TODO not implemented */
    string name; /**< Name of the component (as a string). */
    int count{-1}; /**< Can be used to represent multiple Components with the same properties. By default, it represents only 1 component, and is set to -1. */
    /**
    Component type of the component. The component type denotes of which class the instance is (Often the components are stored as Component*, even though they are a member of one of the child classes)
    \n This attribute is constant, set by the constructor, and READONLY.
    \n SYS_SAGE_COMPONENT_NONE -> class Component
    \n SYS_SAGE_COMPONENT_THREAD -> class Thread
    \n SYS_SAGE_COMPONENT_CORE -> class Core
    \n SYS_SAGE_COMPONENT_CACHE -> class Cache
    \n SYS_SAGE_COMPONENT_SUBDIVISION -> class Subdivision
    \n SYS_SAGE_COMPONENT_NUMA -> class Numa
    \n SYS_SAGE_COMPONENT_CHIP -> class Chip
    \n SYS_SAGE_COMPONENT_MEMORY -> class Memory
    \n SYS_SAGE_COMPONENT_STORAGE -> class Storage
    \n SYS_SAGE_COMPONENT_NODE -> class Node
    \n SYS_SAGE_COMPONENT_TOPOLOGY -> class Topology
    */
    const int componentType;
    vector<Component*> children; /**< Contains the list (std::vector) of pointers to children of the component in the component tree. */
    Component* parent { nullptr }; /**< Contains pointer to the parent component in the component tree. If this component is the root, parent will be NULL.*/
    vector<DataPath*> dp_incoming; /**< Contains references to data paths that point to this component. @see DataPath */
    vector<DataPath*> dp_outgoing; /**< Contains references to data paths that point from this component. @see DataPath */

private:
};

/**
Class Topology - the root of the topology.
\n It is not required to have an instance of this class at the root of the topology. Any component can be the root. This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Topology : public Component {
public:
    /**
    Topology constructor (no automatic insertion in the Component Tree). Sets:
    \n id=>0
    \n name=>"sys-sage Topology"
    \n componentType=>SYS_SAGE_COMPONENT_TOPOLOGY
    */
    Topology();
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Topology() override = default;
private:
};

/**
Class Node - represents a compute node.
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Node : public Component {
public:
    /**
    Node constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _name = name, default "Node"
    @param componentType=>SYS_SAGE_COMPONENT_NODE
    */
    Node(int _id = 0, string _name = "Node");
    /**
    Node constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "Node"
    @param componentType=>SYS_SAGE_COMPONENT_NODE
    */
    Node(Component * parent, int _id = 0, string _name = "Node");
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Node() override = default;
#ifdef CPUINFO
public:
    int RefreshCpuCoreFrequency(bool keep_history = false);
#endif
#ifdef CAT_AWARE //defined in CAT_aware.cpp
public:
    /**
    !!! Only if compiled with CAT_AWARE functionality, only for Intel CPUs !!!
    \n Creates/updates (bidirectional) data paths between all cores (class Thread) and their L3 cache segment (class Cache). The data paths of type SYS_SAGE_DATAPATH_TYPE_L3CAT contain the COS id (attrib with key "CATcos", value is of type uint64_t*) and the open L3 cache ways (attrib with key "CATL3mask", value is of type uint64_t*) to contain the current settings.
    \n Each time the method is called, new DataPath objects get created, so the last one is always the most up-to-date.
    */
    int UpdateL3CATCoreCOS();
#endif

private:
};

/**
Class Memory - represents a memory element. (Could be main memory of different technologies, could be a GPU memory or any other type.)
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Memory : public Component {
public:
    /**
    Memory constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = 0
    @param _name = "Memory"
    @param componentType=>SYS_SAGE_COMPONENT_MEMORY
    */
    Memory();
    /**
    Memory constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = 0
    @param _name = name, default "Memory"
    @param componentType=>SYS_SAGE_COMPONENT_MEMORY
    */
    Memory(Component * parent, int id = 0, string _name = "Memory", long long _size = -1);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Memory() override = default;
    /**
     * Retrieves size/capacity of the memory element
     * @return size
     * @see size
    */
    long long GetSize();
    /**
     * Sets size/capacity of the memory element
     * @param _size = size
    */
    void SetSize(long long _size);
    /**
    @private
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    long long size; /**< size/capacity of the memory element*/
    bool is_volatile; /**< is volatile? */

#ifdef NVIDIA_MIG
public:
    long long GetMIGSize(string uuid = "");
#endif
};

/**
Class Storage - represents a persistent storage device. (Of any kind.)
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Storage : public Component {
public:
    /**
    Storage constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = 0
    @param _name = "Storage"
    @param componentType=>SYS_SAGE_COMPONENT_STORAGE
    */
    Storage();
    /**
    Storage constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = 0
    @param _name = "Storage"
    @param componentType=>SYS_SAGE_COMPONENT_STORAGE
    */
    Storage(Component * parent);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Storage() override = default;
    /**
     * Retrieves size/capacity of the storage device
     * @return size
     * @see size
    */
    long long GetSize();
    /**
     * Sets size/capacity of the storage device
     * @param _size = size
    */
    void SetSize(long long _size);
    /**
    @private
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    long long size; /**< size/capacity of the storage device */
};

/**
Class Chip - represents a building block of a node. It may be a CPU socket, a GPU, a NIC or any other chip.
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Chip : public Component {
public:
    /**
    Chip constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _name = name, default "Chip"
    @param _type = chip type, default SYS_SAGE_CHIP_TYPE_NONE. Defines which chip we are describing. The options are: SYS_SAGE_CHIP_TYPE_NONE (default/generic), SYS_SAGE_CHIP_TYPE_CPU, SYS_SAGE_CHIP_TYPE_CPU_SOCKET, SYS_SAGE_CHIP_TYPE_GPU.
    @param componentType=>SYS_SAGE_COMPONENT_CHIP
    */
    Chip(int _id = 0, string _name = "Chip", int _type = SYS_SAGE_CHIP_TYPE_NONE);
    /**
    Chip constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "Chip"
    @param _type = chip type, default SYS_SAGE_CHIP_TYPE_NONE. Defines which chip we are describing. The options are: SYS_SAGE_CHIP_TYPE_NONE (default/generic), SYS_SAGE_CHIP_TYPE_CPU, SYS_SAGE_CHIP_TYPE_CPU_SOCKET, SYS_SAGE_CHIP_TYPE_GPU.
    @param componentType=>SYS_SAGE_COMPONENT_CHIP
    */
    Chip(Component * parent, int _id = 0, string _name = "Chip", int _type = SYS_SAGE_CHIP_TYPE_NONE);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Chip() override = default;
    /**
    Sets the vendor of the chip.
    @param _vendor - The name of the vendor to set.
    */
    void SetVendor(string _vendor);
    /**
    Gets the vendor of the chip.
    @return The name of the vendor.
    @see vendor
    */
    string GetVendor();
    
    /**
    Sets the model of the chip.
    @param _model - The model name to set.
    */
    void SetModel(string _model);
    
    /**
    Gets the model of the chip.
    @return The model name.
    @see model
    */
    string GetModel();
    
    /**
    Sets the type of the chip.
    @param chipType - The chip type to set.
    */
    void SetChipType(int chipType);
    
    /**
    Gets the type of the chip.
    @return The chip type.
    @see type
    */
    int GetChipType();
    
    /**
    @private
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    string vendor; /**< Vendor of the chip */
    string model; /**< Model of the chip */
    int type; /**< Type of the chip, e.g., CPU, GPU */
#ifdef NVIDIA_MIG
public:
    /**
    Updates the MIG settings for the chip.
    @param uuid - The UUID of the chip, default is an empty string.
    @return Status of the update operation.
    */
    int UpdateMIGSettings(string uuid = "");

    /**
    Gets the number of SMs for the MIG.
    @param uuid - The UUID of the chip, default is an empty string.
    @return The number of SMs.
    */
    int GetMIGNumSMs(string uuid = "");

    /**
    Gets the number of cores for the MIG.
    @param uuid - The UUID of the chip, default is an empty string.
    @return The number of cores.
    */
    int GetMIGNumCores(string uuid = "");
#endif
};

/**
Class Cache - represents a data cache memories in the system (of different levels/purposes).
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Cache : public Component {
public:
    /**
    Cache constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _cache_level - (int) cache level (1=L1, 2=L2, ...), default 0. This value is represented as a cahce_type, i.e. either one of int cache_level or string cache_type can be used.
    @param _cache_size - size of the cache (Bytes), default 0
    @param _associativity - number of cache associativity ways, default -1
    @param _cache_line_size - size of a cache line (Bytes), default -1
    @param componentType=>SYS_SAGE_COMPONENT_CACHE
    */
    Cache(int _id = 0, int  _cache_level = 0, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);
    /**
    Cache constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _cache_type - (string) name/type of the cache (constant, textture, L1, ...). Only one of (int) cache_level or (string) cache_type can be used.
    @param _cache_size - size of the cache (Bytes), default 0
    @param _associativity - number of cache associativity ways, default -1
    @param _cache_line_size - size of a cache line (Bytes), default -1
    @param componentType=>SYS_SAGE_COMPONENT_CACHE
    */
    Cache(Component * parent, int _id, string _cache_type, long long _cache_size = 0, int _associativity = -1, int _cache_line_size = -1);
    /**
    Cache constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _cache_level - (int) cache level (1=L1, 2=L2, ...), default 0. This value is represented as a cahce_type, i.e. either one of int cache_level or string cache_type can be used.
    @param _cache_size - size of the cache (Bytes), default 0
    @param _associativity - number of cache associativity ways, default -1
    @param _cache_line_size - size of a cache line (Bytes), default -1
    @param componentType=>SYS_SAGE_COMPONENT_CACHE
    */
    Cache(Component * parent, int _id = 0, int _cache_level = 0, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Cache() override = default;

    /**
    @returns cache level of this cache, assuming there's only 1 or no digit in the "cache_type" (e.g. "L1", "texture")
    */
    int GetCacheLevel();
    
    /**
    Sets cache level of this cache using integer value (For e.g. "1" for "L1", etc.)
    @param _cache_level - value for cache_type
    */
    void SetCacheLevel(int _cache_level);
    
    /**
    Retrieves cache name of this cache (e.g. "L1", "texture")
    @returns cache name 
    @see cache_name
    */
    string GetCacheName();
    
    /**
    Sets cache name of this cache (e.g. "L1", "texture")
    @param _cache_name - value for cache_type
    */
    void SetCacheName(string _name);

    /**
     * Retrieves size/capacity of the cache
     * @return size
     * @see size
    */
    long long GetCacheSize();
    /**
     * Sets size/capacity of the cache
     * @param _size = size
    */
    void SetCacheSize(long long _cache_size);
    /**
    @returns the number of the cache associativity ways of this cache
    */
    int GetCacheAssociativityWays();

    /**
    Sets cache associativity ways of this cache
    @param _associativity - value for cache_associativity_ways
    */
    void SetCacheAssociativityWays(int _associativity);
    /**
    @returns the size of a cache line of this cache
    */
    int GetCacheLineSize();
    /**
     * Sets the size of a cache line of this cache
    @param _cache_line_size = cache_line_size
    */
    void SetCacheLineSize(int _cache_line_size);
    /**
    @private
    !!Should normally not be caller from the outside!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    string cache_type; /**< cache level or cache type */
    long long cache_size;  /**< size/capacity of the cache */
    int cache_associativity_ways; /**< number of cache associativity ways */
    int cache_line_size; /**< size of a cache line */

#ifdef NVIDIA_MIG
public:
    long long GetMIGSize(string uuid = "");
#endif
};

/**
Class Subdivision - represents a data cache memories in the system (of different levels/purposes).
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Subdivision : public Component {
public:
    /**
    Subdivision constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _name = name, default "Subdivision"
    @param _componentType, componentType, default SYS_SAGE_COMPONENT_SUBDIVISION. If componentType is not SYS_SAGE_COMPONENT_SUBDIVISION or SYS_SAGE_COMPONENT_NUMA, it is set to SYS_SAGE_COMPONENT_SUBDIVISION as default option.
    */
    Subdivision(int _id = 0, string _name = "Subdivision", int _componentType = SYS_SAGE_COMPONENT_SUBDIVISION);
    /**
    Subdivision constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "Subdivision"
    @param _componentType, componentType, default SYS_SAGE_COMPONENT_SUBDIVISION. If componentType is not SYS_SAGE_COMPONENT_SUBDIVISION or SYS_SAGE_COMPONENT_NUMA, it is set to SYS_SAGE_COMPONENT_SUBDIVISION as default option.
    */
    Subdivision(Component * parent, int _id = 0, string _name = "Subdivision", int _componentType = SYS_SAGE_COMPONENT_SUBDIVISION);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Subdivision() override = default;
    /**
     * Sets the type of the subdivision
    @param subdivisionType = type 
    */
    void SetSubdivisionType(int subdivisionType);
    /**
    @returns the type of subdivision
    @see type
    */
    int GetSubdivisionType();
    /**
    @private
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
protected:
    int type; /**< Type of the subdivision. Each user can have his own numbering, i.e. the type is there to identify different types of subdivisions as the user defines it.*/
};

/**
Class Numa - represents a NUMA region on a chip.
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Numa : public Subdivision {
public:
    /**
    Numa constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _name = name, default "Numa"
    @param _size = size or capacity of the NUMA region, default -1, i.e. no value.
    @param componentType=>SYS_SAGE_COMPONENT_NUMA
    */
    Numa(int _id = 0, long long _size = -1);
    /**
    Numa constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "Numa"
    @param _size = size or capacity of the NUMA region, default -1, i.e. no value.
    @param componentType=>SYS_SAGE_COMPONENT_NUMA
    */
    Numa(Component * parent, int _id = 0, long long _size = -1);
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Numa() override = default;
    /**
    Get size of the Numa memory segment.
    @returns size of the Numa memory segment.
    */
    long long GetSize();

    /**
    Set size of the Numa memory segment.
    @param _size - size of the Numa memory segment.
    */
    void SetSize(long long _size);

    /**
    @private 
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    long long size; /**< size of the Numa memory segment.*/
};

/**
Class Core - represents a CPU core, or a GPU streaming multiprocessor.
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Core : public Component {
public:
    /**
    Core constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _name = name, default "Core"
    @param componentType=>SYS_SAGE_COMPONENT_CORE
    */
    Core(int _id = 0, string _name = "Core");
    /**
    Core constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "Core"
    @param componentType=>SYS_SAGE_COMPONENT_CORE
    */
    Core(Component * parent, int _id = 0, string _name = "Core");
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Core() override = default;
private:

#ifdef CPUINFO
public:
    int RefreshFreq(bool keep_history = false);
    void SetFreq(double _freq);
    double GetFreq();
private:
    double freq;
#endif
};

/**
Class Thread - represents HW thread on CPUs, or a thread on a GPU.
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Thread : public Component {
public:
    /**
    Thread constructor (no automatic insertion in the Component Tree). Sets:
    @param _id = id, default 0
    @param _name = name, default "Thread"
    @param componentType=>SYS_SAGE_COMPONENT_THREAD
    */
    Thread(int _id = 0, string _name = "Thread");
    /**
    Thread constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
    @param parent = the parent 
    @param _id = id, default 0
    @param _name = name, default "Thread"
    @param componentType=>SYS_SAGE_COMPONENT_THREAD
    */    
    Thread(Component * parent, int _id = 0, string _name = "Thread");
    /**
    * @private
    * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
    */
    ~Thread() override = default;

#ifdef CPUINFO //defined in cpuinfo.cpp
public:
    int RefreshFreq(bool keep_history = false);
    double GetFreq();
#endif

#ifdef CAT_AWARE //defined in CAT_aware.cpp
public:
        /**
        !!! Only if compiled with CAT_AWARE functionality, only for Intel CPUs !!!
        \n Retrieves the L3 cache size available to this thread. This size is retrieved based on the last update with UpdateL3CATCoreCOS() -- i.e. you should call that method before.
        @returns Available L3 cache size in bytes.
        @see int UpdateL3CATCoreCOS();
        */
        long long GetCATAwareL3Size();
#endif
private:
};

#endif

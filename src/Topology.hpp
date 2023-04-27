#ifndef TOPOLOGY
#define TOPOLOGY

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "DataPath.hpp"
#include <libxml/parser.h>


#define SYS_SAGE_COMPONENT_NONE 1
#define SYS_SAGE_COMPONENT_THREAD 2
#define SYS_SAGE_COMPONENT_CORE 4
#define SYS_SAGE_COMPONENT_CACHE 8
#define SYS_SAGE_COMPONENT_SUBDIVISION 16
#define SYS_SAGE_COMPONENT_NUMA 32
#define SYS_SAGE_COMPONENT_CHIP 64
#define SYS_SAGE_COMPONENT_MEMORY 128
#define SYS_SAGE_COMPONENT_STORAGE 256
#define SYS_SAGE_COMPONENT_NODE 512
#define SYS_SAGE_COMPONENT_TOPOLOGY 1024

#define SYS_SAGE_SUBDIVISION_TYPE_NONE 1
#define SYS_SAGE_SUBDIVISION_TYPE_GPU_SM 2

#define SYS_SAGE_CHIP_TYPE_NONE 1
#define SYS_SAGE_CHIP_TYPE_CPU 2
#define SYS_SAGE_CHIP_TYPE_CPU_SOCKET 4
#define SYS_SAGE_CHIP_TYPE_GPU 8


using namespace std;
class DataPath;

/**
Generic class Component - all components inherit from this class, i.e. this class defines attributes and methods common to all components.
\n Therefore, these can be used universally among all components. Usually, a Component instance would be an instance of one of the child classes, but a generic component (instance of class Component) is also possible.
*/
class Component {
public:

    /**
    Generic constructor. Sets:
    \n id=>0
    \n name=>"unknown"
    \n componentType=>SYS_SAGE_COMPONENT_NONE
    */
    Component();
    Component(Component * parent);
    /**
    Generic Component constructor. Usually should not be called. Sets:
    @param _id = id
    @param _name = name
    @param _componentType = componentType
    */
    Component(int _id, string _name, int _componentType);
    Component(Component * parent, int _id, string _name, int _componentType);

    /**
    Inserts a Child component to this component (in the Component Tree).
    The child pointer will be inserted at the end of std::vector of children (retrievable through GetChildren(), GetChild(int _id) etc.)
    @param child - a pointer to a Component (or any class instance that inherits from Component).
    @see GetChildren()
    @see GetChild(int _id)
    */
    void InsertChild(Component * child);
    /**
    //TODO
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
    Retrieve a Componet* to a child with child.id=_id.
    \n Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
    */
    Component* GetChild(int _id);
    Component* GetChildByType(int _componentType);
    vector<Component*> GetAllChildrenByType(int _componentType);

    /**
    Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
    \n Returns first occurence that matches these criteria.
    @param _id - the id to look for
    @param _componentType - the component type where to look for the id
    @return Component * matching the criteria. NULL if no match found
    */
    Component* FindSubcomponentById(int _id, int _componentType);
    void FindAllSubcomponentsByType(vector<Component*>* outArray, int _componentType);
    /**
    Moves up the tree until a parent of given type.
    @param _componentType - the desired component type
    @return Component * matching the criteria. NULL if no match found
    */
    Component* FindParentByType(int _componentType);
    /**
    Returns the number of Components of type SYS_SAGE_COMPONENT_THREAD in the subtree.
    */
    int GetNumThreads();
    /**
    Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
    \n 0=leaf, 1=children are leaves, 2=at most children's children are leaves .....
    @return maximal distance to a leaf
    */
    int GetTopoTreeDepth();//0=empty, 1=1element,...
    /**
    Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
    \n E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
    @param depth - how many levels down the tree should be looked
    @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
    */
    void GetComponentsNLevelsDeeper(vector<Component*>* outArray, int depth);
    /**
    Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in each std::vector children.
    \n E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
    @param depth - how many levels down the tree should be looked
    @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)

    */
    void GetSubcomponentsByType(vector<Component*>* outArray, int componentType);
    /**
    Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. The tree is traversed DFS in order as the children are stored in each std::vector children.
    @param componentType - componentType
    @param outArray - output parameter (vector with results)
        \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
    */
    void GetSubtreeNodeList(vector<Component*>* outArray);

    /**
    Returns the DataPaths of this component according to their orientation.
    @param orientation - either SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING
    @return Pointer to std::vector<DataPath *> with the result (dp_outgoing on SYS_SAGE_DATAPATH_OUTGOING, or dp_incoming on SYS_SAGE_DATAPATH_INCOMING, otherwise NULL)
    @see dp_incoming
    @see dp_outgoing
    */
    vector<DataPath*>* GetDataPaths(int orientation);
    /**
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
    DataPath* GetDpByType(int dp_type, int orientation);
    /**
    Retrieves all DataPath * from the list of this component's data paths with matching type and orientation.
    Results are returned in vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
    @param dp_type - DataPath type (dp_type) to search for.
    @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
    @param outDpArr - output parameter (vector with results)
        \n An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
        \n The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
    */
    void GetAllDpByType(vector<DataPath*>* outDpArr, int dp_type, int orientation);
    int CheckComponentTreeConsistency();
    /**
    Calculates approximate memory footprint of the subtree of this element (including the relevant data paths).
    @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
    @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
    @return The total size in bytes
    */
    int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
    /**
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
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();

    /**
    TODO this part
    */
    map<string,void*> attrib;
protected:

    int id; /**< Numeric ID of the component. There is no requirement for uniqueness of the ID, however it is advised to have unique IDs at least in the realm of parent's children. Some tree search functions, which take the id as a search parameter search for first match, so the user is responsible to manage uniqueness in the realm of the search subtree (or should be aware of the consequences of not doing so). Component's ID is set by the constructor, and is retrieved via int GetId(); */
    int depth; /**< TODO not implemented */
    string name; /**< Name of the component (as a string). */
    int count; /**< Can be used to represent multiple Components with the same properties. By default, it represents only 1 component, and is set to -1. */
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
    Component* parent; /**< Contains pointer to the parent component in the component tree. If this component is the root, parent will be NULL.*/
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
    Generic constructor. Sets:
    \n id=>0
    \n name=>"topology"
    \n componentType=>SYS_SAGE_COMPONENT_TOPOLOGY
    */
    Topology();
private:
};

/**
Class Node - represents a compute node.
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Node : public Component {
public:
    /**
    Generic constructor. Sets:
    \n id=>0
    \n name=>"sys-sage node"
    \n componentType=>SYS_SAGE_COMPONENT_NODE
    */
    Node();
    Node(Component* parent);
    /**
    Node constructor. Sets:
    \n id=>_id
    \n name=>"sys-sage node"
    \n componentType=>SYS_SAGE_COMPONENT_NODE
    */
    Node(int _id);
    Node(Component* parent, int _id);
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
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Memory"
    \n componentType=>SYS_SAGE_COMPONENT_MEMORY
    */
    Memory();
    Memory(Component * parent);
    Memory(Component * parent, string _name);
    Memory(Component * parent, string _name, long long _size);
    long long GetSize();
    void SetSize(long long _size);
    /**
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    long long size; /**< size/capacity of the memory element*/
    bool is_volatile; /**< is volatile? */
};

/**
Class Storage - represents a persistent storage device. (Of any kind.)
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Storage : public Component {
public:
    /**
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Storage"
    \n componentType=>SYS_SAGE_COMPONENT_STORAGE
    */
    Storage();
    Storage(Component * parent);
    long long GetSize();
    void SetSize(long long _size);
    /**
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
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Chip"
    \n componentType=>SYS_SAGE_COMPONENT_CHIP
    */
    Chip();
    /**
    Generic constructor. Sets:
    \n id=>_id
    \n name=>"Chip"
    \n componentType=>SYS_SAGE_COMPONENT_CHIP
    */
    Chip(int _id);
    Chip(int _id, string _name);
    Chip(int _id, string _name, int _type);
    Chip(Component * parent);
    Chip(Component * parent, int _id);
    Chip(Component * parent, int _id, string _name);
    Chip(Component * parent, int _id, string _name, int _type);

    void SetVendor(string _vendor);
    string GetVendor();
    void SetModel(string _model);
    string GetModel();
    void SetChipType(int chipType);
    int GetChipType();
    /**
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    string vendor;
    string model;
    int type;
};

/**
Class Cache - represents a data cache memories in the system (of different levels/purposes).
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Cache : public Component {
public:
    /**
    Generic constructor. Sets:
    \n id=>0
    \n name=>"cache"
    \n componentType=>SYS_SAGE_COMPONENT_CACHE
    */
    Cache();
    /**
    Cache component constructor. The name and componentType are set as in Cache() constructor
    @param _id - id of the Cache
    @param _cache_level - (int) cache level (1=L1, 2=L2, ...)
    @param _cache_size - size of the cache (Bytes?)
    @param _associativity - number of cache associativity ways
    @see Cache()
    */
    Cache(int _id, int  _cache_level, unsigned long long _cache_size, int _associativity);
    Cache(int _id, int  _cache_level, unsigned long long _cache_size, int _associativity, int _cache_line_size);
    Cache(int _id, string  _cache_type, unsigned long long _cache_size, int _associativity, int _cache_line_size);
    Cache(Component * parent);
    Cache(Component * parent, int _id, string _cache_type);
    Cache(Component * parent, int _id, int  _cache_level, unsigned long long _cache_size, int _associativity);
    Cache(Component * parent, int _id, int  _cache_level, unsigned long long _cache_size, int _associativity, int _cache_line_size);
    Cache(Component * parent, int _id, string _cache_type, unsigned long long _cache_size, int _associativity, int _cache_line_size);

    /**
    @returns cache level of this cache
    */
    int GetCacheLevel();
    string GetCacheName();
    /**
    @returns cache size of this cache
    */
    long long GetCacheSize();
    void SetCacheSize(long long _cache_size);
    /**
    @returns the number of the cache associativity ways of this cache
    */
    int GetCacheAssociativityWays();
    /**
    @returns the size of a cache line of this cache
    */
    int GetCacheLineSize();
    void SetCacheLineSize(int _cache_line_size);
    /**
    !!Should normally not be used!! Helper function of XML dump generation.
    @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
    */
    xmlNodePtr CreateXmlSubtree();
private:
    string cache_type; /**< cache level or cache type */
    long long cache_size;  /**< size/capacity of the cache */
    int cache_associativity_ways; /**< number of cache associativity ways */
    int cache_line_size; /**< size of a cache line */
};

/**
Class Subdivision - represents a data cache memories in the system (of different levels/purposes).
\n This class is a child of Component class, therefore inherits its attributes and methods.
*/
class Subdivision : public Component {
public:
    /**
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Subdivision"
    \n componentType=>SYS_SAGE_COMPONENT_SUBDIVISION
    */
    Subdivision();
    /**
    Subdivision constructor. Sets
    \n name=>"Subdivision"
    \n componentType=>SYS_SAGE_COMPONENT_SUBDIVISION
    @param _id - id of the component
    */
    Subdivision(int _id);
    Subdivision(int _id, string _name);
    Subdivision(int _id, int _componentType);
    /**
    Helper constructor for inherited classes of Subdivision. Usually, this constructor will not be called. If called, the _componentType must be SYS_SAGE_COMPONENT_SUBDIVISION.
    @param _id - id of the component
    @param _name - name
    @param _componentType - always use SYS_SAGE_COMPONENT_SUBDIVISION
    */
    Subdivision(int _id, string _name, int _componentType);
    Subdivision(Component * parent);
    Subdivision(Component * parent, int _id);
    Subdivision(Component * parent, int _id, string _name);
    Subdivision(Component * parent, int _id, int _componentType);
    Subdivision(Component * parent, int _id, string _name, int _componentType);

    void SetSubdivisionType(int subdivisionType);
    int GetSubdivisionType();
    /**
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
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Numa"
    \n componentType=>SYS_SAGE_COMPONENT_NUMA
    */
    Numa();
    /**
    Numa constructor. Sets
    \n name=>"Numa"
    \n componentType=>SYS_SAGE_COMPONENT_NUMA
    @param _id - id of the component
    */
    Numa(int _id);
    /**
    Numa constructor. Sets
    \n name=>"Numa"
    \n componentType=>SYS_SAGE_COMPONENT_NUMA
    @param _id - id of the component
    @param _size - size of the numa region of the memory (in case the memory segment is not represented by class Memory)
    */
    Numa(int _id, int _size);
    Numa(Component * parent);
    Numa(Component * parent, int _id);
    Numa(Component * parent, int _id, long long _size);
    /**
    Get size of the Numa memory segment.
    @returns size of the Numa memory segment.
    */
    long long GetSize();

    /**
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
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Core"
    \n componentType=>SYS_SAGE_COMPONENT_CORE
    */
    Core();
    /**
    Core constructor. Sets
    \n name=>"Core"
    \n componentType=>SYS_SAGE_COMPONENT_CORE
    @param _id - id of the component
    */
    Core(int _id);
    Core(int _id, string _name);
    Core(Component * parent);
    Core(Component * parent, int _id);
    Core(Component * parent, int _id, string _name);
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
    Generic constructor. Sets:
    \n id=>0
    \n name=>"Thread"
    \n componentType=>SYS_SAGE_COMPONENT_THREAD
    */
    Thread();
    /**
    Thread constructor. Sets
    \n name=>"Thread"
    \n componentType=>SYS_SAGE_COMPONENT_THREAD
    @param _id - id of the component
    */
    Thread(int _id);
    Thread(int _id, string _name);
    Thread(Component * parent);
    Thread(Component * parent, int _id);
    Thread(Component * parent, int _id, string _name);

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

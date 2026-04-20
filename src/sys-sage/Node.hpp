#ifndef NODE_HPP
#define NODE_HPP

#include "Component.hpp"

namespace sys_sage {

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
        @param componentType=>sys_sage::ComponentType::Node
        */
        Node(int _id = 0, std::string _name = "Node");
        /**
        Node constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Node"
        @param componentType=>sys_sage::ComponentType::Node
        */
        Node(Component * parent, int _id = 0, std::string _name = "Node");
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        ~Node() override = default;
    #ifdef PROC_CPUINFO
    public:
        /**
         * Refreshes the CPU core frequency of the node.
         * @param keep_history - If true, the history of the CPU core frequency will be kept.
         */
        int RefreshCpuCoreFrequency(bool keep_history = false);
    #endif
    #ifdef INTEL_PQOS //defined in intel_pqos.cpp
    public:
        /**
        \n Creates/updates (bidirectional) data paths between all cores (class Thread) and their L3 cache segment (class Cache). The data paths of type SYS_SAGE_DATAPATH_TYPE_L3CAT contain the COS id (attrib with key "CATcos", value is of type uint64_t*) and the open L3 cache ways (attrib with key "CATL3mask", value is of type uint64_t*) to contain the current settings.
        \n Each time the method is called, new DataPath objects get created, so the last one is always the most up-to-date.
        
        Note: This function is defined only when sys-sage is compiled with INTEL_PQOS functionality (only for Intel CPUs).
        */
        int UpdateL3CATCoreCOS();
    #endif

    private:
    };

}

#endif //NODE_HPP
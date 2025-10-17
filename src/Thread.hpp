#ifndef THREAD_HPP
#define THREAD_HPP

#include "Component.hpp"

namespace sys_sage {


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
        Thread(int _id = 0, std::string _name = "Thread");
        /**
        Thread constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Thread"
        @param componentType=>SYS_SAGE_COMPONENT_THREAD
        */    
        Thread(Component * parent, int _id = 0, std::string _name = "Thread");
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        ~Thread() override = default;

    #ifdef PROC_CPUINFO //defined in proc_cpuinfo.cpp
    public:
        /**
        * Refreshes the frequency of the thread.
        */
        int RefreshFreq(bool keep_history = false);

        /**
        * Gets the frequency of the thread.
        */
        double GetFreq();
    #endif

    #ifdef INTEL_PQOS //defined in intel_pqos.cpp
    public:
            /**
            !!! Only if compiled with INTEL_PQOS functionality, only for Intel CPUs !!!
            \n Retrieves the L3 cache size available to this thread. This size is retrieved based on the last update with UpdateL3CATCoreCOS() -- i.e. you should call that method before.
            @returns Available L3 cache size in bytes.
            @see int UpdateL3CATCoreCOS();
            */
            long long GetCATAwareL3Size();
    #endif
    private:
    };

}
#endif //THREAD_HPP


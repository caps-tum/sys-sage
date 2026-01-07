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

#ifdef SS_PAPI
        /**
         * @brief Get the perf counter value of a specific event in an event set.
         *
         * @param event The event of interest.
         * @param eventSet The event set to which the event belongs to.
         * @param timestamp An optional parameter used to select a perf counter
         *                  value from a specific perf counter reading. A value
         *                  of 0 refers to the latest reading.
         *
         * @return > 0 if a perf counter value exists for the given paramters, 0 otherwise.
         */
        long long GetPAPImetric(int event, int eventSet, unsigned long long timestamp = 0) const;

        /**
         * @brief Print all PAPI metrics of this CPU in given the event set.
         *
         * @param eventSet Specifies the event set of interest. If the value is
         *        `PAPI_NULL`, then all event sets will be printed.
         */
        void PrintPAPImetrics(int eventSet) const;

        /**
         * @brief Retrieve the relation that involves this component and corresponds to the given event set.
         *
         * @param eventSet The event set of interest.
         *
         * @return A valid pointer if such a relation exists, `nullptr` otherwise.
         */
        Relation *GetPAPIrelation(int eventSet) const;

        /**
         * @brief Retrieve all relations that are associated to this component
         *        and contain PAPI metrics.
         *
         * @return A vector containing the relations. It may be empty if no
         *         such relations exist.
         */
        std::vector<Relation *> FindPAPIrelations() const;

        /**
         * @brief Retrieve all relations that are associated to this component
         *        and contain PAPI metrics.
         *
         * @param A vector used for storing the relations.
         */
        void FindPAPIrelations(std::vector<Relation *> &) const;

        /**
         * @brief Retrieve all eventSets that are associated to this component.
         *
         * @return A vector containing the relations. It may be empty if no
         *         such relations exist.
         */
        std::vector<int> FindPAPIeventSets() const;

        /**
         * @brief Retrieve all eventSets that are associated to this component.
         *
         * @param A vector used for storing the relations.
         */
        void FindPAPIeventSets(std::vector<int> &) const;

#endif
    private:
    };

}
#endif //THREAD_HPP


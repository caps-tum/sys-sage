#ifndef TOPOLOGY_HPP
#define TOPOLOGY_HPP

#include "Component.hpp"

namespace sys_sage {

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
}

#endif
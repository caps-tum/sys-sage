#ifndef TOPOLOGY_HPP
#define TOPOLOGY_HPP

#include <sys-sage/Component.hpp>

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
         *
         * @brief Initializes a JSON object that represents this component.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        void _ToJson(nlohmann::json &obj) const override;

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
        int _FromJson(const nlohmann::json &obj) override;

    private:
    };
}

#endif
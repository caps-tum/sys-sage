#ifndef TOPOLOGY_HPP
#define TOPOLOGY_HPP

#include <sys-sage/Component.hpp>

namespace sys_sage {

    /**
     * @brief Represents the root of the topology.
     * It is not required to have an instance of this class at the root of the topology. Any component can be the root.
     */
    class Topology : public Component {
    public:
        /**
        Topology constructor (no automatic insertion in the Component Tree).

        Sets componentType to sys_sage::ComponentType::Topology.

        @pythonBinding
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
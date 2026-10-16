#ifndef CORE_HPP
#define CORE_HPP

#include <sys-sage/Component.hpp>

namespace sys_sage {

    /**
     * @brief Represents a CPU core.
     */
    class Core : public Component {
    public:
        /**
         * @brief Core constructor (no automatic insertion in the Component Tree).
         *        Sets componentType to sys_sage::ComponentType::Core.
         *
         * @param _id The ID of the core (default 0).
         * @param _name The name of the core (default "Core").
         *
         * @pythonBinding
         */
        Core(int _id = 0, const std::string &_name = "Core");

        /**
         * @brief Core constructor with insertion into the Component Tree as the parent's child.
         *        Sets componentType to sys_sage::ComponentType::Core.
         *
         * @param parent The parent of this core within the tree.
         * @param _id The ID of the core (default 0).
         * @param _name The name of the core (default "Core").
         *
         * @pythonBinding
         */
        Core(Component * parent, int _id = 0, const std::string &_name = "Core");

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

    #ifdef PROC_CPUINFO
        /**
         * @brief Refreshes the frequency of the core.
         *
         * @param keep_history Whether the new frequency metric should be also be saved in a dedicated vector.
         *
         * @return 0 on success, 1 on failure.
         *
         * @pythonBinding
         */
        int RefreshFreq(bool keep_history = false);

        /**
        * Sets the frequency of the core.
        */
        void SetFreq(double _freq);

        /**
        * Gets the frequency of the core.
        */
        double GetFreq() const;
    private:
        double freq; ///< The currently measured frequency of the core.
    #endif
    };
}

#endif //CORE_HPP

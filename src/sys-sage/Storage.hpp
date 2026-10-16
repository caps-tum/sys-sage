#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <sys-sage/Component.hpp>

namespace sys_sage {

    /**
     * @brief Represents a persistent storage device (of any kind).
     */
    class Storage : public Component {
    public:
        /**
        Storage constructor (no automatic insertion in the Component Tree). Sets:
        @param _size = size/capacity of the storage device, default -1

        Sets componentType to sys_sage::ComponentType::Storage.

        @pythonBinding
        */
        Storage(long long _size = -1);
        /**
        Storage constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _size = size/capacity of the storage device, default -1

        Sets componentType to sys_sage::ComponentType::Storage.

        @pythonBinding
        */
        Storage(Component * parent, long long _size = -1);

        /**
         * Retrieves size/capacity of the storage device
         * @return size
         * @see size
        */
        long long GetSize() const;
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
        xmlNodePtr _CreateXmlSubtree() override;

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
        long long size; /**< size/capacity of the storage device */
    };
}

#endif //STORAGE_HPP
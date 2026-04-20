#ifndef STORAGE_HPP
#define STORAGE_HPP

#include "Component.hpp"

namespace sys_sage {

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
        @param _size = size/capacity of the storage device, default -1
        */
        Storage(long long _size = -1);
        /**
        Storage constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = 0
        @param _name = "Storage"
        @param componentType=>SYS_SAGE_COMPONENT_STORAGE
        @param _size = size/capacity of the storage device, default -1
        */
        Storage(Component * parent, long long _size = -1);
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
    private:
        long long size; /**< size/capacity of the storage device */
    };
}

#endif //STORAGE_HPP
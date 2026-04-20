
#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "Component.hpp"

namespace sys_sage {

    /**
    Class Memory - represents a memory element. (Could be main memory of different technologies, could be a GPU memory or any other type.)
    \n This class is a child of Component class, therefore inherits its attributes and methods.
    */
    class Memory : public Component {
    public:
        /**
        Memory constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = 0
        @param _name = "Memory"
        @param componentType=>SYS_SAGE_COMPONENT_MEMORY
        @param _size = size/capacity of the memory element, default -1
        @param is_volatile = true if the memory is volatile, default false
        */
        Memory(long long _size = -1, bool is_volatile = false);
        /**
        Memory constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = 0
        @param _name = name, default "Memory"
        @param componentType=>SYS_SAGE_COMPONENT_MEMORY
        @param _size = size/capacity of the memory element, default -1
        @param is_volatile = true if the memory is volatile, default false
        */
        Memory(Component * parent, int id = 0, std::string _name = "Memory", long long _size = -1, bool is_volatile = false);
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        ~Memory() override = default;
        /**
         * Retrieves size/capacity of the memory element
         * @return size
         * @see size
        */
        long long GetSize() const;
        /**
         * Sets size/capacity of the memory element
         * @param _size = size
        */
        void SetSize(long long _size);
        /**
         * Retrieves if the memory element is volatile
         * @return is_volatile  
         * 
         */
        bool GetIsVolatile() const;
        /**
         * Sets if the memory element is volatile
         * @param _is_volatile = update is_volatile
         */
        void SetIsVolatile(bool _is_volatile);
        /**
        @private
        !!Should normally not be used!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        xmlNodePtr _CreateXmlSubtree() override;
    private:
        long long size; /**< size/capacity of the memory element*/
        bool is_volatile; /**< is volatile? */

    #ifdef NVIDIA_MIG
    public:
        /**
         * Gets the MIG size of the memory element.
         */
        long long GetMIGSize(std::string uuid = "") const;
    #endif
    };

}
#endif //MEMORY_HPP
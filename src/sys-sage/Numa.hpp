#ifndef NUMA_HPP
#define NUMA_HPP

#include <sys-sage/Subdivision.hpp>

namespace sys_sage {

    /**
    Class Numa - represents a NUMA region on a chip.
    \n This class is a child of Subdivision (which is a child of Component) class, therefore inherits its attributes and methods.
    */
    class Numa : public Subdivision {
    public:
        /**
        Numa constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "Numa"
        @param _size = size or capacity of the NUMA region, default -1, i.e. no value.
        @param componentType=>SYS_SAGE_COMPONENT_NUMA
        */
        Numa(int _id = 0, long long _size = -1);
        /**
        Numa constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Numa"
        @param _size = size or capacity of the NUMA region, default -1, i.e. no value.
        @param componentType=>SYS_SAGE_COMPONENT_NUMA
        */
        Numa(Component * parent, int _id = 0, long long _size = -1);

        /**
        Get size of the Numa memory segment.
        @returns size of the Numa memory segment.
        */
        long long GetSize() const;

        /**
        Set size of the Numa memory segment.
        @param _size - size of the Numa memory segment.
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
        long long size; /**< size of the Numa memory segment.*/
    };
}

#endif //NUMA_HPP
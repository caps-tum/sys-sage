#ifndef SUBDIVISION_HPP
#define SUBDIVISION_HPP

#include <sys-sage/Component.hpp>

namespace sys_sage {

    /**
     * @brief Represents a logical or physical collection of components that share topological semantics (e.g. NUMA node or GPU streaming multiprocessor).
     */
    class Subdivision : public Component {
    public:
        /**
        Subdivision constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "Subdivision"

        Sets componentType to sys_sage::ComponentType::Subdivision.

        @pythonBinding
        */
        Subdivision(int _id = 0, const std::string &_name = "Subdivision");
        /**
        Subdivision constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Subdivision"

        Sets componentType to sys_sage::ComponentType::Subdivision.

        @pythonBinding
        */
        //SVDOCTODO check all the API documentation. where there is SYS_SAGE_COMPONENT_xxx, replace it by matching sys_sage::ComponentType::xxx
        Subdivision(Component * parent, int _id = 0, const std::string &_name = "Subdivision");

        /**
         * Sets the category of the subdivision
        @param subdivisionCategory = category 
        */
        void SetSubdivisionCategory(SubdivisionCategory::type subdivisionCategory);
        /**
        @returns the category of subdivision
        @see category
        */
        SubdivisionCategory::type GetSubdivisionCategory() const;
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

    protected:
        /**
        Subdivision constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "Subdivision"
        @param _componentType The type of the subdivision (often set to sys_sage::ComponentType::Numa).
        */
        //SVDOCTODO
        Subdivision(int _id, const std::string &_name, ComponentType::type _componentType);
        /**
        Subdivision constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Subdivision"
        @param _componentType The type of the subdivision (often set to sys_sage::ComponentType::Numa).
        */
        //SVDOCTODO
        Subdivision(Component * parent, int _id, const std::string &_name, ComponentType::type _componentType);

        SubdivisionCategory::type category; /**< Category of the subdivision. Each user can have his own numbering, i.e. the category is there to identify different categories of subdivisions as the user defines it.*/
    };
}

#endif //SUBDIVISION_HPP
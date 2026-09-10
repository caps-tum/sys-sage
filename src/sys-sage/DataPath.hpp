#ifndef DATAPATH_CPP
#define DATAPATH_CPP

#include <sys-sage/Relation.hpp>

namespace sys_sage { //forward declaration
    class Component;
}

namespace sys_sage {
    /**
     * @class DataPath
     * @brief Represents a data path (arbitrary relation or data movement) between two Components in the topology.
     *
     * DataPaths form a data-path graph, which is orthogonal to the Component Tree. Each Component contains references
     * to all DataPaths going to or from it, in addition to its parent and children in the Component Tree. This enables
     * navigation between Components and access to data stored in Components or DataPaths.
     *
     * DataPaths can be oriented (directed) or bidirectional, and can carry additional information such as bandwidth,
     * latency, and user-defined categories. They are essential for modeling memory hierarchies, interconnects, and other
     * logical or physical data flows in heterogeneous systems.
     */
    class DataPath : public Relation {

    public:
        /**
         * @private
         *
         * @brief DataPath default constructor.
         * @param _dp_category Type of the DataPath.
         * @param _bw Bandwidth from the source (provides the data) to the target (requests the data)
         * @param _latency Data load latency from the source (provides the data) to the target (requests the data)
         */
        DataPath(DataPathCategory::type _dp_category = DataPathCategory::None, double _bw = -1, double _latency = -1);

        /**
         * @brief DataPath constructor.
         * @param _source Pointer to the source Component. (If _oriented == DataPathOrientation::Bidirectional, there is no difference between _source and _target)
         * @param _target Pointer to the target Component.
         * @param _oriented Is the Data Path oriented? (DataPathOrientation::Oriented = oriented DP; DataPathOrientation::Bidirectional = NOT oriented DP)
         * @param _dp_category (optional) Category of the Data Path (user-defined, default: DataPathCategory::None).
         *        Predefined categories: DataPathCategory::None, DataPathCategory::Logical, DataPathCategory::Physical, DataPathCategory::Datatransfer, DataPathCategory::L3CAT, DataPathCategory::MIG, DataPathCategory::C2C .
         *        Each user can define an arbitrary category in the DataPathCategory namespace.
        */
        DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathCategory::type _dp_category = sys_sage::DataPathCategory::None);
        /**
         * @brief DataPath constructor. DataPath category is set to DataPathCategory::None.
         * @param _source Pointer to the source Component. (If _oriented == DataPathOrientation::Bidirectional, there is no difference between _source and _target)
         * @param _target Pointer to the target Component.
         * @param _oriented Is the Data Path oriented? (DataPathOrientation::Oriented = oriented DP; DataPathOrientation::Bidirectional = NOT oriented DP)
         * @param _bw Bandwidth from the source (provides the data) to the target (requests the data)
         * @param _latency Data load latency from the source (provides the data) to the target (requests the data)
         */
        DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, double _bw, double _latency);

        /**
         * @brief DataPath constructor.
         * @param _source Pointer to the source Component. (If _oriented == DataPathOrientation::Bidirectional, there is no difference between _source and _target)
         * @param _target Pointer to the target Component.
         * @param _oriented Is the Data Path oriented? (DataPathOrientation::Oriented = oriented DP; DataPathOrientation::Bidirectional = NOT oriented DP)
         * @param _dp_category (optional) Category of the Data Path (user-defined, default: DataPathCategory::None).
         *        Predefined categories: DataPathCategory::None, DataPathCategory::Logical, DataPathCategory::Physical, DataPathCategory::Datatransfer, DataPathCategory::L3CAT, DataPathCategory::MIG, DataPathCategory::C2C .
         *        Each user can define an arbitrary category in the DataPathCategory namespace.
         * @param _bw Bandwidth from the source (provides the data) to the target (requests the data)
         * @param _latency Data load latency from the source (provides the data) to the target (requests the data)
         */
        DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathCategory::type _dp_category, double _bw, double _latency);

        /**
         * @brief Retrieves the source Component.
         * @returns Pointer to the source Component
         */
        Component* GetSource() const;
        /**
         * @brief Retrieves the target Component.
         * @returns Pointer to the target Component
         */
        Component* GetTarget() const;
        /**
         * @brief Retrieves the bandwidth from the source to the target.
         * @returns Bandwidth from the source(provides the data) to the target(requests the data)
         */
        double GetBandwidth() const;
        /**
         * @brief Sets the bandwidth from the source to the target.
         * @param _bandwidth Value for bw
         * @see bw
         */
        void SetBandwidth(double _bandwidth);
        /**
        @returns Data load latency from the source(provides the data) to the target(requests the data)
        */
        double GetLatency() const;
        /**
         * @brief Sets the data load latency from the source to the target.
         * @param _latency Value for latency
         * @see latency
         */
        void SetLatency(double _latency);

        /**
         * @brief Updates the source of the DataPath. The datapath is removed from the outgoing/incoming vectors of the old source and 
         * added to the vectors of the new source.
         * @param _new_source New source of the DataPath.
         */
        int UpdateSource(Component * _new_source);

        /**
         * @brief Updates the target of the DataPath. The datapath is removed from the outgoing/incoming vectors of the old target and 
         * added to the vectors of the new target.
         * @param _new_target New target of the DataPath.
         */
        int UpdateTarget(Component * _new_target);
        
        /**
         * @returns Category of the Data Path (DataPathCategory). Do not mix with RelationType!
         * @see category
         */
        DataPathCategory::type GetDataPathCategory() const;

        /**
         * @brief Retrieves the orientation of the DataPath (DataPathOrientation::Oriented or DataPathOrientation::Bidirectional).
         * @returns orientation 
         * @see oriented
         */
        DataPathOrientation::type GetOrientation() const;

        /**
         * @brief Prints basic information about the Data Path to stdout.
         * Prints componentType and Id of the source and target Components, the bandwidth, load latency, and the attributes.
         * For each attribute, the name and value are printed; the value is only retyped to uint64_t (so will print nonsensical values for other data types).
         */
        void Print() const override;
        /**
         * @private
         * @brief Helper function for XML export.
         * Should normally not be used directly. Used internally for exporting the DataPath to XML.
         * @return Pointer to the created XML entry node.
         */
        xmlNodePtr _CreateXmlEntry() override;

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this relation.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        void _ToJson(nlohmann::json &obj) const override;

        /**
         * @private
         *
         * @brief Initializes this relation through JSON. Intended for internal
         *        use.
         *
         * @param obj The JSON object containing the data.
         * @param componentMap A map used for the relation graph.
         *
         * @return 0 on success, 1 otherwise.
         */
        int _FromJson(const nlohmann::json &obj,
                      const std::unordered_map<uintptr_t, Component *> &componentMap) override;

    private:
        DataPathCategory::type dp_category; /**< category of the datapath */

        double bw; /**< Bandwidth from the source(provides the data) to the target(requests the data) */
        double latency; /**< Data load latency from the source(provides the data) to the target(requests the data) */

    };
 
} //namespace sys_sage 
#endif // RELATION_HPP

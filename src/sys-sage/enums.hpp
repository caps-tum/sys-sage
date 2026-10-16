/**
 * @file
 *
 * @brief Definition of contants used for indicating the type or categroy of components and relations.
 *        All constants are grouped into various namespaces that can be extended with additional, user-defined constants.
 */

#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>
#include <unordered_map>

namespace sys_sage {

/////////////////////////////////////////////////////////////
////////////////////// COMPONENT ENUMS //////////////////////
/////////////////////////////////////////////////////////////

    /**
     * @namespace sys_sage::ComponentType
     *
     * @brief Enumerates Component types.
     *        Used to distinguish between different hardware and logical components in the topology.
     *        The types match the respective C++ classes.
     */
    namespace ComponentType{
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Any = -1; /**< Any type of component. */
        constexpr type Generic = 1; /**< Component class. */
        constexpr type Thread = 2; /**< Thread class. */
        constexpr type Core = 3; /**< Core class. */
        constexpr type Cache = 4; /**< Cache class. */
        constexpr type Subdivision = 5; /**< Subdivision class. */
        constexpr type Numa = 6; /**< Numa class. */
        constexpr type Chip = 7; /**< Chip class. */
        constexpr type Memory = 8; /**< Memory class. */
        constexpr type Storage = 9; /**< Storage class. */
        constexpr type Node = 10; /**< Node class. */
        constexpr type QuantumBackend = 11; /**< QuantumBackend class. */
        constexpr type AtomSite = 12; /**< AtomSite class. */
        constexpr type Qubit = 13; /**< Qubit class. */
        constexpr type Topology = 14; /**< Topology class. */

        //SVTODO this should remain private???
        static const std::unordered_map<type, const std::string> names = {
            {Generic, "GenericComponent"},
            {Thread, "HW_Thread"},
            {Core, "Core"},
            {Cache, "Cache"},
            {Subdivision, "Subdivision"},
            {Numa, "NUMA"},
            {Chip, "Chip"},
            {Memory, "Memory"},
            {Storage, "Storage"},
            {Node, "Node"},
            {QuantumBackend, "QuantumBackend"},
            {AtomSite, "AtomSite"},
            {Qubit, "Qubit"},
            {Topology, "Topology"}
        };

        /**
         * @private
         * @brief Converts a ComponentType value to a human-readable string.
         * Use Component::GetComponentTypeStr() for a more convenient way to get the string representation.
         * @param rt ComponentType value
         * @return String representation of the component type
         * 
         */
        inline const std::string &ToString(type rt) {
            static const std::string unknown ("Unknown");

            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return unknown;
        }
    }

    /**
     * @namespace sys_sage::SubdivisionCategory
     *
     * @brief Enumerates Subdivision categories.
     *        Provides semantic differences between subdivisions with no connection to the data type of the component.
     */
    namespace SubdivisionCategory {
        using type = int32_t; /**< Data type of the constants. */

        constexpr type None = 1; /**< Generic subdivision. */
        constexpr type GpuSM = 2; /**< GPU SM subdivision. */
    }

    /**
     * @namespace sys_sage::ChipCategory
     *
     * @brief Enumerates Chip categories.
     *        Provides semantic differences between subdivisions with no connection to the data type of the component.
     */
    namespace ChipCategory {
        using type = int32_t; /**< Data type of the constants. */

        constexpr type None = 1; /**< Generic chip. */
        constexpr type Cpu = 2; /**< CPU chip. */
        constexpr type CpuSocket = 3; /**< CPU socket. */
        constexpr type Gpu = 4; /**< GPU chip. */
    }

////////////////////////////////////////////////////////////
////////////////////// RELATION ENUMS //////////////////////
////////////////////////////////////////////////////////////

    /**
     * @namespace sys_sage::RelationType
     *
     * @brief Enumerates Relation types.
     *        Used to model different kinds of relationships between components.
     *        The types match the respective C++ classes.
     */
    namespace RelationType{
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Any = -1; /**< Any type of relation. */
        constexpr type Relation = 0; /**< Relation class. */
        constexpr type DataPath = 1; /**< DataPath class. */
        constexpr type QuantumGate = 2; /**< QuantumGate class. */
        constexpr type CouplingMap = 3; /**< CouplingMap class. */
        constexpr type _num_relation_types = 4; /**< The number of predefined relation types. */

        constexpr type RelationTypeList [_num_relation_types] = {
            Relation, 
            DataPath, 
            QuantumGate, 
            CouplingMap,
        }; /**< A list of all predefined relation types. */

        //SVTODO this should remain private???
        static const std::unordered_map<type, const std::string> names = {
            {Any, "Any"},
            {Relation, "Relation"},
            {DataPath, "DataPath"},
            {QuantumGate, "QuantumGate"},
            {CouplingMap, "CouplingMap"}
        };

        /**
         * @private
         * @brief Converts a RelationType value to a human-readable string.
         * Use Relation::GetRelationTypeStr() for a more convenient way to get the string representation.
         * @param rt RelationType value
         * @return String representation of the relation type
         */
        inline const std::string &ToString(type rt) {
            static const std::string unknown ("Unknown");
          
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return unknown;
        }
    }

    /**
     * @namespace sys_sage::RelationCategory
     *
     * @brief Enumerates Relation categories (not to confuse with RelationType).
     *        Provides semantic differences between relations with no connection to the data type of the component.
     *
     *        Due to the high versatility of the Relation class and the numerous ways
     *        in which components can be related to one another, sys-sage provides the
     *        means for distinguishing different information carried and represented by
     *        relations (e.g. performance metrics collection).
     */
    namespace RelationCategory {
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Any = -1; /**< Any relation category. */
        constexpr type Default = 0; /**< Default relation category. */
#ifdef SS_PAPI
        constexpr type PAPI_Metrics = 1; /**< A relation capturing PAPI metrics. */
#endif
    }

    /**
     * @namespace sys_sage::DataPathCategory
     *
     * @brief Enumerates DataPath categories.
     *        Provides semantic differences between data paths with no connection to the data type of the component.
     */
    namespace DataPathCategory{
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Any = -1; /**< Any data path category. */
        constexpr type None = 0; /**< Generic data path. */
        constexpr type Logical = 1; /**< A data path describing a logical connection between two components. */ // TODO: isn't this already handled by the generic Relation class -> maybe remove this from the data path?
        constexpr type Physical = 2; /**< A data path describing a physical/hardware connection of two components. */
        constexpr type Datatransfer = 3; /**< A data path describing data transfer attributes. */
        constexpr type L3CAT = 4; /**< A data path describing cache partitioning settings. */
        constexpr type MIG = 5; /**< A data path describing NVIDIA GPU partitioning settings. */
        constexpr type C2C = 6; /**< A data path describing cache-to-cache latencies. */
    }

    /**
     * @namespace sys_sage::DataPathDirection
     *
     * @brief Enumerates the directionality for DataPath objects.
     */
    namespace DataPathDirection{
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Any = 1; /**< Any direction. */
        constexpr type Outgoing = 2; /**< The first component is the source and the second the target. */
        constexpr type Incoming = 3; /**< The first component is the target and the second the source. */
    }

    //SVTODO rename this to RelationOrientation? oriented x not oriented and include this also into general Relation, rather than just DataPath?
    /**
     * @namespace sys_sage::DataPathOrientation
     *
     * @brief Enumerates the orientation for DataPath objects.
     */
    namespace DataPathOrientation{
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Oriented = 1; /**< The data path is directed from source to target. */
        constexpr type Bidirectional = 2; /**< The data path is bidirectional between source and target. */
    }

    /**
     *
     * @namespace sys_sage::QuantumGateCategory
     *
     * @brief Enumerates QuantumGate categories.
     *        Provides semantic differences between quantum gates with no connection to the data type of the component.
     */
    namespace QuantumGateCategory{
        using type = int32_t; /**< Data type of the constants. */

        constexpr type Unknown = 0; /**< Unknown gate. */
        constexpr type Id = 1; /**< Identity gate. */
        constexpr type X = 2; /**< X (Not) gate. */
        constexpr type Rz = 3; /**< RZ gate. */
        constexpr type Cnot = 4; /**< CNOT gate. */
        constexpr type Sx = 5; /**< SX gate. */
        constexpr type Toffoli = 6; /**< Toffoli gate. */

        //SVTODO this should remain private???
        static const std::unordered_map<type, const char*> names = {
            {Unknown, "Unknown"},
            {Id, "Id"},
            {X, "X"},
            {Rz, "Rz"},
            {Cnot, "Cnot"},
            {Sx, "Sx"},
            {Toffoli, "Toffoli"}
        };

        /**
         * @private
         *
         * @brief Converts a QuantumGateCategory value to a human-readable string.
         * //TODO: Use QuantumGate::GetQuantumGateCategoryStr() for a more convenient way to get the string representation.
         * @param rt QuantumGateCategory value
         * @return String representation of the quantum gate type
         */
        inline const char* ToString(type rt) {
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return "Unknown";
        }
    }
}
#endif //ENUMS_HPP
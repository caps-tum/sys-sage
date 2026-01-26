#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>
#include <unordered_map>


namespace sys_sage {

/////////////////////////////////////////////////////////////
////////////////////// COMPONENT ENUMS //////////////////////
/////////////////////////////////////////////////////////////

    /**
     * @namespace ComponentType
     * @brief Enumerates all supported component types in sys-sage.
     *
     * Used to distinguish between different hardware and logical components in the topology.
     * The type alias 'type' is always int32_t for consistency and extensibility.
     * Use ComponentType::type for all component type variables and arguments.
     */
    namespace ComponentType{
        using type = int32_t; /**< ComponentType datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Any = -1;
        constexpr type Generic = 1; /**< class Component (do not use normally)*/
        [[ deprecated("Use ComponentType::Generic instead. This constant will be removed in the future (used up until version 1.0.0).") ]]
        constexpr type None = Generic; /**< class Component (do not use normally)*/
        constexpr type Thread = 2; /**< class Thread */
        constexpr type Core = 3; /**< class Core */
        constexpr type Cache = 4; /**< class Cache */
        constexpr type Subdivision = 5; /**< class Subdivision */
        constexpr type Numa = 6; /**< class Numa */
        constexpr type Chip = 7; /**< class Chip */
        constexpr type Memory = 8; /**< class Memory */
        constexpr type Storage = 9; /**< class Storage */
        constexpr type Node = 10; /**< class Node */
        constexpr type QuantumBackend = 11; /**< class QuantumBackend */
        constexpr type AtomSite = 12; /**< class AtomSite */
        constexpr type Qubit = 13; /**< class Qubit */
        constexpr type Topology = 14; /**< class Topology */

        //SVTODO this should remain private???
        static const std::unordered_map<type, const char*> names = {
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
        inline const char* ToString(type rt) {
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return "Unknown";
        }
    }

    /**
     * @namespace SubdivisionType
     * @brief Enumerates subdivision types for components (e.g., GPU SMs).
     */
    namespace SubdivisionType {
        using type = int32_t; /**< SubdivisionType datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type None = 1; /**< Generic Subdivision type. */
        constexpr type GpuSM = 2; /**< Subdivision type for GPU SMs */
    }

    /**
     * @namespace ChipType
     * @brief Enumerates chip types (CPU, GPU, etc.).
     */
    namespace ChipType {
        using type = int32_t; /**< ChipType datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type None = 1; /**< Generic Chip type. */
        constexpr type Cpu = 2; /**< Chip type used for a CPU. */
        constexpr type CpuSocket = 3; /**< Chip type used for one CPU socket. */
        constexpr type Gpu = 4; /**< Chip type used for a GPU.*/
    }

////////////////////////////////////////////////////////////
////////////////////// RELATION ENUMS //////////////////////
////////////////////////////////////////////////////////////

    /**
     * @namespace RelationType
     * @brief Enumerates all supported relation types in sys-sage.
     *
     * Used to distinguish between different types of relations (edges) in the topology graph.
     * It gives a high-level classification of the relationship between hardware components
     * (e.g. data transfer between cache and core). Each type corresponds to one C++ class.
     */
    namespace RelationType{
        using type = int32_t; /**< RelationType datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Any = -1;
        constexpr type Relation = 0;
        constexpr type DataPath = 1;
        constexpr type QuantumGate = 2;
        constexpr type CouplingMap = 3;
        constexpr type _num_relation_types = 4;

        constexpr type RelationTypeList [_num_relation_types] = {
            Relation, 
            DataPath, 
            QuantumGate, 
            CouplingMap,
        };

        //SVTODO this should remain private???
        static const std::unordered_map<type, const char*> names = {
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
        inline const char* ToString(type rt) {
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return "Unknown";
        }
    }

    /**
     * @brief Enumerates all supported relation categories in sys-sage.
     *
     * Due to the high versatility of the `Relation` class and the numerous ways
     * in which components can be related to one another, sys-sage provides the
     * means for a more fine grained categorization of the relations. It can be
     * used to indicate a special purpose of the relation (e.g. performance metrics collection).
     */
    namespace RelationCategory {
        using type = int32_t; /**< RelationCategory datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Any = -1; /**< Any category. */
        constexpr type Default = 0; /**< The default category. */
#ifdef SS_PAPI
        constexpr type PAPI_Metrics = 1; /**< A relation used for capturing PAPI metrics. */
#endif
    }

    /**
     * @namespace DataPathType
     * @brief Enumerates types of DataPaths (logical, physical, etc.).
     *
     * Used to specify the semantics of a DataPath between components.
     */
    namespace DataPathType{
        using type = int32_t; /**< DataPathType datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Any = -1;
        constexpr type None = 0; /**< Generic type of DataPath */
        constexpr type Logical = 1; /**< DataPath describes a logical connection/relation of two Components. */
        constexpr type Physical = 2; /**< DataPath describes a physical/hardware connection/relation of two Components. */
        constexpr type Datatransfer = 3; /**< DataPath type describing data transfer attributes. */
        constexpr type L3CAT = 4; /**< DataPath type describing Cache partitioning settings. */
        constexpr type MIG = 5; /**< DataPath type describing GPU partitioning settings. */
        constexpr type C2C = 6; /**< DataPath type describing cache-to-cache latencies (cccbench data source). */
    }
    /**
     * @namespace DataPathDirection
     * @brief Enumerates directionality for DataPaths.
     */
    namespace DataPathDirection{
        using type = int32_t; /**< DataPathDirection datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Any = 1;
        constexpr type Outgoing = 2; /**< This Component is the source DataPath. */
        constexpr type Incoming = 3; /**< This Component is the target DataPath. */
    }

    //SVTODO rename this to RelationOrientation? oriented x not oriented and include this also into general Relation, rather than just DataPath?
    /**
     * @namespace DataPathOrientation
     * @brief Enumerates orientation (directed/bidirectional) for DataPaths.
     */
    namespace DataPathOrientation{
        using type = int32_t; /**< DataPathOrientation datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Oriented = 1; /**< DataPath is directed from the source to the target. */
        constexpr type Bidirectional = 2; /**< DataPath has no direction. */
    }
    /**
     * @namespace QuantumGateType
     * @brief Enumerates quantum gate types.
     */
    namespace QuantumGateType{
        using type = int32_t; /**< QuantumGateType datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type Unknown = 0; /**< Unknown Gate */
        constexpr type Id = 1; /**< Identity Gate */
        constexpr type X = 2; /**< X (Not) Gate */
        constexpr type Rz = 3; /**< RZ Gate */
        constexpr type Cnot = 4; /**< CNOT Gate */
        constexpr type Sx = 5; /**< SX Gate */
        constexpr type Toffoli = 6; /**< Toffoli Gate */

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
         * @brief Converts a QuantumGateType value to a human-readable string.
         * //TODO: Use QuantumGate::GetQuantumGateTypeStr() for a more convenient way to get the string representation.
         * @param rt QuantumGateType value
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
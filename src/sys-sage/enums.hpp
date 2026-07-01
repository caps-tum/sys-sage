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
     * @namespace SubdivisionCategory
     * @brief Enumerates subdivision categories for components (e.g., GPU SMs).
     *        Provides finer difference between subdivisions with no connection
     *        to the data type of the component.
     */
    namespace SubdivisionCategory {
        using type = int32_t; /**< SubdivisionCategory datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type None = 1; /**< Generic Subdivision category. */
        constexpr type GpuSM = 2; /**< Subdivision type for GPU SMs */
    }

    /**
     * @namespace ChipCategory
     * @brief Enumerates chip types (CPU, GPU, etc.). Provides finer difference
     *        between chips with no connection to the data type of the
     *        component.
     */
    namespace ChipCategory {
        using type = int32_t; /**< ChipCategory datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

        constexpr type None = 1; /**< Generic Chip category. */
        constexpr type Cpu = 2; /**< Chip category used for a CPU. */
        constexpr type CpuSocket = 3; /**< Chip category used for one CPU socket. */
        constexpr type Gpu = 4; /**< Chip category used for a GPU.*/
    }

////////////////////////////////////////////////////////////
////////////////////// RELATION ENUMS //////////////////////
////////////////////////////////////////////////////////////

    /**
     * @namespace RelationType
     * @brief Enumerates all supported relation types (different data types) in sys-sage.
     *
     * It marks the data type of the relation (e.g. sys_sage::Relation,
     * sys_sage::DataPath, sys_sage::CouplingMap, ...) -- each relation type has
     * different attributes, functionalities and API.
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
     * @brief Enumerates all supported relation categories (standalone attribute,
     *        not to confuse with `RelationType`) in sys-sage.
     *
     * Due to the high versatility of the `Relation` class and the numerous ways
     * in which components can be related to one another, sys-sage provides the
     * means for distingishing different information carried and represented by
     * relations (e.g. performance metrics collection). As opposed to
     * `RelationType`, `RelationCategory` has no connection to the data type,
     * but instead expresses what information this object carries.
     *
     * Users can extend the `RelationCategoty` namespace to define their own
     * categories, reflecting the various use-case-specific objects.
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
     * @namespace DataPathCategory
     * @brief Enumerates categories of DataPaths (logical, physical, etc.).
     *        Provides finer difference between datapaths with no connection
     *        to the data type of the relation.
     *
     * Used to specify the semantics of a DataPath between components.
     */
    namespace DataPathCategory{
        using type = int32_t; /**< DataPathCategory datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

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
     * @namespace QuantumGateCategory
     * @brief Enumerates quantum gate types. Provides finer difference between
     *        quantum gates with no connection to the data type of the relation.
     */
    namespace QuantumGateCategory{
        using type = int32_t; /**< QuantumGateCategory datatype -- to indicate a parameter should be from this enum/namespace (as there are no hard restrictions from C++). */

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
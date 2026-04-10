set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake" ${CMAKE_MODULE_PATH})

#defines.hpp header with #cmakedefine 
configure_file(${PROJECT_SOURCE_DIR}/src/defines.hpp.in ${PROJECT_SOURCE_DIR}/src/defines.hpp)

include_directories(external_interfaces)
set(EXT_INTF "external_interfaces")
set(PY_BINDS "python-bindings")

set(SOURCES
    Component.cpp
    Thread.cpp
    Core.cpp
    Cache.cpp
    Subdivision.cpp
    Numa.cpp
    Chip.cpp
    Memory.cpp
    Storage.cpp
    Node.cpp
    QuantumBackend.cpp
    Qubit.cpp
    AtomSite.cpp
    Topology.cpp
    Relation.cpp
    DataPath.cpp
    QuantumGate.cpp
    CouplingMap.cpp
    xml_dump.cpp
    xml_load.cpp
    ${EXT_INTF}/intel_pqos.cpp
    ${EXT_INTF}/proc_cpuinfo.cpp
    ${EXT_INTF}/nvidia_mig.cpp
    #${PY_BINDS}/sys-sage-bindings.cpp
    parsers/hwloc.cpp
    parsers/caps-numa-benchmark.cpp
    parsers/mt4g.cpp
    parsers/mt4g_v0_1.cpp
    parsers/cccbench.cpp
    parsers/qdmi-parser.cpp
    parsers/iqm-parser.cpp
    )

set(HEADERS
    sys-sage.hpp
    defines.hpp
    enums.hpp
    Component.hpp
    Thread.hpp
    Core.hpp
    Cache.hpp
    Subdivision.hpp
    Numa.hpp
    Chip.hpp
    Memory.hpp
    Storage.hpp
    Node.hpp
    QuantumBackend.hpp
    Qubit.hpp
    AtomSite.hpp
    Topology.hpp
    Relation.hpp
    DataPath.hpp
    QuantumGate.hpp
    CouplingMap.hpp
    xml_dump.hpp
    xml_load.hpp
    parsers/hwloc.hpp
    parsers/caps-numa-benchmark.hpp
    parsers/mt4g.hpp
    parsers/cccbench.hpp
    parsers/qdmi-parser.hpp
    parsers/iqm-parser.hpp
    )

if(SS_PAPI)
	list(APPEND SOURCES ${EXT_INTF}/ss_papi.cpp)
	list(APPEND HEADERS ${EXT_INTF}/ss_papi.hpp)
endif()

# add_library(sys-sage SHARED ${SOURCES} ${HEADERS})

if(PYBIND)
    add_subdirectory(python)
endif()

    add_library(sys-sage SHARED ${SOURCES} ${HEADERS})

    # For Quantum Systems
    if(QDMI)
        target_link_libraries(sys-sage PUBLIC
            backend_ibm
            qdmi
            qinfo
        )
    endif()

    target_link_libraries(sys-sage PUBLIC nlohmann_json::nlohmann_json)

    target_include_directories(sys-sage PUBLIC  
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  
        $<INSTALL_INTERFACE:inc>
        $<INSTALL_INTERFACE:lib>
    )
    install(
        TARGETS sys-sage
        EXPORT sys-sage-targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/cmake/lib
        ARCHIVE DESTINATION lib/cmake/lib
    )
    #for spack
    install(
        EXPORT sys-sage-targets
        FILE sys-sage-targets.cmake
        DESTINATION lib/cmake/syssage
        NAMESPACE syssage::
    )
    install(DIRECTORY "."
        DESTINATION lib/cmake/inc
        FILES_MATCHING PATTERN "*.hpp")

    install(
        TARGETS sys-sage
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    install(DIRECTORY "."
        DESTINATION inc
        FILES_MATCHING PATTERN "*.hpp")

#endif()

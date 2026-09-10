# header with #cmakedefine macros for build options
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sys-sage/defines.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/sys-sage/defines.hpp)

######################################################
#################### SOURCE FILES ####################
######################################################

target_sources(sys-sage PRIVATE
    sys-sage/Component.cpp
    sys-sage/Thread.cpp
    sys-sage/Core.cpp
    sys-sage/Cache.cpp
    sys-sage/Subdivision.cpp
    sys-sage/Numa.cpp
    sys-sage/Chip.cpp
    sys-sage/Memory.cpp
    sys-sage/Storage.cpp
    sys-sage/Node.cpp
    sys-sage/QuantumBackend.cpp
    sys-sage/Qubit.cpp
    sys-sage/AtomSite.cpp
    sys-sage/Topology.cpp

    sys-sage/Relation.cpp
    sys-sage/DataPath.cpp
    sys-sage/QuantumGate.cpp
    sys-sage/CouplingMap.cpp

    sys-sage/attribute.cpp
    sys-sage/xml_dump.cpp
    sys-sage/xml_load.cpp
    sys-sage/json_serialization.cpp

    sys-sage/parsers/hwloc.cpp
    sys-sage/parsers/caps-numa-benchmark.cpp
    sys-sage/parsers/mt4g.cpp
    sys-sage/parsers/mt4g_v0_1.cpp
    sys-sage/parsers/cccbench.cpp
    sys-sage/parsers/iqm-parser.cpp
)

if(INTEL_PQOS)
    target_sources(sys-sage PRIVATE sys-sage/external_interfaces/intel_pqos.cpp)
endif()

if(NVIDIA_MIG)
    target_sources(sys-sage PRIVATE sys-sage/external_interfaces/nvidia_mig.cpp)
endif()

if(PROC_CPUINFO)
    target_sources(sys-sage PRIVATE sys-sage/external_interfaces/proc_cpuinfo.cpp)
endif()

if(PAPI)
    target_sources(sys-sage PRIVATE sys-sage/external_interfaces/ss_papi.cpp)
endif()

if(QDMI)
    target_sources(sys-sage PRIVATE sys-sage/external_interfaces/qdmi-parser.cpp)
endif()

######################################################
################ INCLUDES & LINKING ##################
######################################################

include(GNUInstallDirs)

target_include_directories(sys-sage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(sys-sage PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(sys-sage PUBLIC LibXml2::LibXml2)

if(NVIDIA_MIG)
    target_link_libraries(sys-sage PUBLIC CUDA::nvml)
endif()

if(PAPI)
    target_include_directories(sys-sage PUBLIC ${PAPI_INCLUDE_DIRS})
    target_link_libraries(sys-sage PUBLIC ${PAPI_LIBRARIES})
endif()

if(QDMI)
    target_link_libraries(sys-sage PUBLIC
        backend_ibm
        qdmi
        qinfo
    )
endif()

######################################################
#################### INSTALLATION ####################
######################################################

install(
    TARGETS sys-sage
    EXPORT sys-sage-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES sys-sage.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY sys-sage/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sys-sage
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
)

install(
    EXPORT sys-sage-targets
    FILE sys-sage-targets.cmake
    NAMESPACE sys-sage::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/defines.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/defines.hpp
)

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
    external_interfaces/intel_pqos.cpp
    external_interfaces/proc_cpuinfo.cpp
    external_interfaces/nvidia_mig.cpp
    parsers/hwloc.cpp
    parsers/caps-numa-benchmark.cpp
    parsers/mt4g.cpp
    parsers/mt4g_v0_1.cpp
    parsers/cccbench.cpp
    parsers/qdmi-parser.cpp
    parsers/iqm-parser.cpp
)

if(SS_PAPI)
	list(APPEND SOURCES external_interfaces/ss_papi.cpp)
endif()

add_library(sys-sage SHARED ${SOURCES})

target_compile_features(sys-sage PUBLIC cxx_std_20)
set_target_properties(sys-sage PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(sys-sage PRIVATE -Wall)

include(GNUInstallDirs)

target_include_directories(sys-sage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/sys-sage>
)

find_package(nlohmann_json 3.10 REQUIRED)
target_link_libraries(sys-sage PUBLIC nlohmann_json::nlohmann_json)

find_package(LibXml2 REQUIRED)
target_link_libraries(sys-sage PRIVATE LibXml2::LibXml2)

if(NVIDIA_MIG)
    find_package(CUDAToolkit 10.0 REQUIRED)

    target_include_directories(sys-sage CUDA::nvml)
    target_link_libraries(sys-sage CUDA::nvml)
endif()

if(QDMI)
    find_package(backends REQUIRED)
    find_package(qinfo REQUIRED)

    target_link_libraries(sys-sage PUBLIC
        backend_ibm
        qdmi
        qinfo
    )
endif()

if(PAPI)
    find_package(PAPI REQUIRED)

    target_include_directories(sys-sage PUBLIC ${PAPI_INCLUDE_DIRS})
    target_link_libraries(sys-sage ${PAPI_LIBRARIES})
endif()

install(TARGETS sys-sage
    EXPORT sys-sage-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sys-sage
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT sys-sage-targets
    FILE sys-sage-targets.cmake
    NAMESPACE sys_sage::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/pkg/sys-sage-config.cmake.in
    ${PROJECT_BINARY_DIR}/sys-sage-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)
install(FILES ${PROJECT_BINARY_DIR}/sys-sage-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)

configure_file(
    ${PROJECT_SOURCE_DIR}/pkg/sys-sage.pc.in
    ${PROJECT_BINARY_DIR}/sys-sage.pc
    @ONLY
)
install(FILES ${PROJECT_BINARY_DIR}/sys-sage.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

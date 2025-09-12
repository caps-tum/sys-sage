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

if(PAPI_METRICS)
	list(APPEND SOURCES ${EXT_INTF}/papi.cpp)
	list(APPEND HEADERS ${EXT_INTF}/papi.hpp)
endif()

# add_library(sys-sage SHARED ${SOURCES} ${HEADERS})

if(PY_SYS_SAGE)
    # find_package(Python 3.13 COMPONENTS Interpreter Development REQUIRED)
    link_directories(${PYTHON_LIBRARY_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS})
    set(PYBIND11_FINDPYTHON ON)
    # find_package(pybind11 CONFIG REQUIRED)
    # Get Python site-packages path
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_paths().get('purelib', ''))"
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    #set install directory of the python library
    if(CMAKE_INSTALL_PREFIX STREQUAL "")
        # use install prefix if result is empty
        if(PYTHON_SITE STREQUAL "")
            set(PYTHON_SITE "${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
            message(WARNING "PYTHON_SITE was empty, falling back to: ${PYTHON_SITE}")
        # else()
            # message(STATUS "Python site-packages directory: ${PYTHON_SITE}")
        endif()
    else()
        set(PYTHON_SITE "${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
        # message(STATUS "Python site-packages directory: ${PYTHON_SITE}")
    endif()

    # set(PY_BINDS ${CMAKE_SOURCE_DIR}/src/python-bindings)
    message(STATUS "PY_BINDS=${PY_BINDS}")

    # This builds a Python extension module (NOT a regular .so/.dylib)
    pybind11_add_module(sys_sage MODULE ${PY_BINDS}/sys-sage-bindings.cpp ${SOURCES} ${HEADERS})
    target_link_libraries(sys_sage PUBLIC ${PYTHON_LIBRARIES} pybind11::module)
    target_link_libraries(sys_sage PUBLIC nlohmann_json::nlohmann_json)
    install(
        TARGETS sys_sage
        LIBRARY DESTINATION ${PYTHON_SITE}       # For Unix-like systems
        RUNTIME DESTINATION ${PYTHON_SITE}       # For Windows (safe to include)
    )
    message(STATUS "Python site-packages directory: ${PYTHON_SITE}")

endif()
#else()

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

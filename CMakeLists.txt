cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.22)

set (CMAKE_CXX_STANDARD 20)
set (CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS " -Wall")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
    message(FATAL_ERROR "Insufficient gcc version (at least 8.0.0) set for example with CXX=/path/to/compiler cmake ....")
  endif()
endif()

project(sys-sage VERSION 1.0.0 LANGUAGES C CXX )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

######################################################
#################### DEPENDENCIES ####################
######################################################
# libxml2
find_package(LibXml2 REQUIRED) # Use pkg-config via the LibXml2 find-module
execute_process(COMMAND xml2-config --cflags OUTPUT_VARIABLE LIBXML_F OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND xml2-config --libs OUTPUT_VARIABLE LIBXML_L OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${LIBXML_F})
link_libraries(${LIBXML_L})
include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${LIBXML2_INCLUDE_DIRS})
link_libraries(${LIBXML2_LIBRARY})
link_libraries(${LIBXML2_LIBRARIES})

# Find nlohmann_json package
find_package(nlohmann_json 3.10 REQUIRED)

if(NVIDIA_MIG)
  find_package(CUDAToolkit 10.0 REQUIRED)
  include_directories(CUDA::nvml)
  link_libraries(CUDA::nvml)
endif()

if(DATA_SOURCES OR DS_HWLOC)
  find_package(HWLOC REQUIRED)
endif()

if(PY_SYS_SAGE)
  find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)
  find_package(pybind11 CONFIG REQUIRED)
endif()

if(QDMI)
    # Find QDMI package
    find_package(backends REQUIRED)
    # Find QInfo package
    find_package(qinfo REQUIRED)
endif()

if(DATA_SOURCES OR DS_NUMA)
    find_package(numactl REQUIRED)
endif()

# Create and install package configuration and version files.
include(CMakePackageConfigHelpers)
configure_package_config_file(${sys-sage_SOURCE_DIR}/pkg/sys-sage-config.cmake.in
                              ${sys-sage_BINARY_DIR}/pkg/sys-sage-config.cmake
                              INSTALL_DESTINATION lib/cmake/sys-sage)
#configure_package_config_file(${sys-sage_SOURCE_DIR}/pkg/sys-sage-config-version.cmake.in
#                              ${sys-sage_BINARY_DIR}/pkg/sys-sage-config-version.cmake
#                              INSTALL_DESTINATION lib/cmake/sys-sage)
install(FILES ${sys-sage_BINARY_DIR}/pkg/sys-sage-config.cmake
                #${sys-sage_BINARY_DIR}/pkg/sys-sage-config-version.cmake
                DESTINATION lib/cmake/sys-sage)

configure_file(${sys-sage_SOURCE_DIR}/pkg/sys-sage.pc.in
                ${sys-sage_BINARY_DIR}/pkg/sys-sage.pc @ONLY)
install(FILES ${sys-sage_BINARY_DIR}/pkg/sys-sage.pc
         DESTINATION lib)



###Options:
option(INTEL_PQOS "Build and install functionality regarding Intel L3 CAT" OFF)
option(NVIDIA_MIG "Build and install functionality regarding NVidia MIG(multi-instance GPU, ampere or newer)" OFF)
option(PROC_CPUINFO "Build and install functionality regarding Linux cpuinfo" OFF)
option(DATA_SOURCES "Build and install all data sources" OFF)
option(DS_HWLOC "Build and install data source hwloc (Retrieves hwloc topology information)" OFF)
option(DS_MT4G "Build and install data source mt4g (Compute and memory topology of NVidia GPUs)" OFF)
option(DS_NUMA "Build and install data source caps-numa-benchmark" OFF)
option(QDMI "Build with QDMI for Quantum Systems" OFF)
option(PAPI "Build and install with performance metrics functionality using PAPI" OFF)

set(SS_PAPI ${PAPI})

option(TEST "Build tests" OFF)
option(TEST_ASAN "Build tests with enabled address sanitizers" OFF)
option(TEST_TSAN "Build tests with enabled thread sanitizers" OFF)
option(TEST_UBSAN "Build tests with enabled undefined behaviour sanitizers" OFF)
option(TEST_COVERAGE "Build tests with enabled coverage" OFF)

if(DATA_SOURCES)
    set(DS_HWLOC ON)
    set(DS_MT4G ON)
    set(DS_NUMA ON)
endif()

if(PAPI)
  find_package(PAPI REQUIRED)
  include_directories(${PAPI_INCLUDE_DIRS})
  link_libraries(${PAPI_LIBRARIES})
endif()

# Top-level build just includes subdirectories.
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(data-sources)

if(${TEST})
    add_subdirectory(test)
endif()

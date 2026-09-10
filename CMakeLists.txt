cmake_minimum_required(VERSION 3.22)
project(sys-sage VERSION 1.1.0 LANGUAGES C CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

######################################################
####################### TARGET #######################
######################################################

add_library(sys-sage SHARED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.1")
        message(FATAL_ERROR "Expected GCC version to be 12.1 or higher, but was ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0.0")
        message(FATAL_ERROR "Expected Clang version to be 16.0.0 or higher, but was ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
endif()

target_compile_features(sys-sage PUBLIC cxx_std_20)
set_target_properties(sys-sage PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(sys-sage PRIVATE -Wall)

######################################################
####################### OPTIONS ######################
######################################################

option(INTEL_PQOS "Build with Intel L3 CAT support" OFF)
option(NVIDIA_MIG "Build with NVIDIA MIG support" OFF)
option(PROC_CPUINFO "Build with Linux proc/cpuinfo support" OFF)
option(DATA_SOURCES "Build with all data sources" OFF)
option(DS_HWLOC "Build with the hwloc data source" OFF)
option(DS_MT4G "Build with the mt4g data source" OFF)
option(DS_NUMA "Build with the caps-numa-benchmark data source" OFF)
option(QDMI "Build with QDMI support" OFF)
option(PAPI "Build with PAPI support" OFF)

set(SS_PAPI ${PAPI}) # for internal use in CXX source code

if(DATA_SOURCES)
    set(DS_HWLOC ON)
    set(DS_MT4G ON)
    set(DS_NUMA ON)
endif()

######################################################
#################### DEPENDENCIES ####################
######################################################

find_package(LibXml2 REQUIRED)
find_package(nlohmann_json 3.10 REQUIRED)

if(NVIDIA_MIG)
    find_package(CUDAToolkit 10.0 REQUIRED)
endif()

if(DS_HWLOC)
    find_package(HWLOC REQUIRED)
endif()

if(DS_NUMA)
    find_package(numactl REQUIRED)
endif()

if(QDMI)
    find_package(backends REQUIRED)
    find_package(qinfo REQUIRED)
endif()

if(PAPI)
    find_package(PAPI REQUIRED)
endif()

######################################################
################### SUBDIRECTORIES ###################
######################################################

add_subdirectory(src)
add_subdirectory(data-sources)

######################################################
###################### PACKAGING #####################
######################################################

include(CMakePackageConfigHelpers)

# CMake
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/pkg/sys-sage-config.cmake.in
    ${PROJECT_BINARY_DIR}/sys-sage-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)

install(
    FILES ${PROJECT_BINARY_DIR}/sys-sage-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)

# pkg-config
configure_file(
    ${PROJECT_SOURCE_DIR}/pkg/sys-sage.pc.in
    ${PROJECT_BINARY_DIR}/sys-sage.pc
    @ONLY
)

install(
    FILES ${PROJECT_BINARY_DIR}/sys-sage.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

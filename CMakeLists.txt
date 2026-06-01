cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.22)

project(sys-sage VERSION 1.0.0 LANGUAGES CXX)

option(INTEL_PQOS "Build with Intel L3 CAT support" OFF)
option(NVIDIA_MIG "Build with NVIDIA MIG support" OFF)
option(PROC_CPUINFO "Build with Linux proc/cpuinfo support" OFF)
option(DATA_SOURCES "Build with all data sources" OFF)
option(DS_HWLOC "Build with the hwloc data source" OFF)
option(DS_MT4G "Build with the mt4g data source" OFF)
option(DS_NUMA "Build with the caps-numa-benchmark data source" OFF)
option(QDMI "Build with QDMI support" OFF)
option(PAPI "Build with PAPI support" OFF)

set(SS_PAPI ${PAPI})

if(DATA_SOURCES)
    set(DS_HWLOC ON)
    set(DS_MT4G ON)
    set(DS_NUMA ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(nlohmann_json 3.10 REQUIRED)
find_package(LibXml2 REQUIRED)

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

add_subdirectory(src)
add_subdirectory(data-sources)

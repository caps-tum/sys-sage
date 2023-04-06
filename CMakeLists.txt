cmake_policy(SET CMP0048 NEW)
project(sys-sage VERSION 0.3.0 LANGUAGES C CXX )

cmake_minimum_required(VERSION 3.22)
set (CMAKE_CXX_STANDARD 20)
set (CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS " -Wall -O0")

find_package(LibXml2 REQUIRED) # Use pkg-config via the LibXml2 find-module

execute_process(COMMAND xml2-config --cflags OUTPUT_VARIABLE LIBXML_F OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND xml2-config --libs OUTPUT_VARIABLE LIBXML_L OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${LIBXML_F})
link_libraries(${LIBXML_L})

include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${LIBXML2_INCLUDE_DIRS})
link_libraries(${LIBXML2_LIBRARY})
link_libraries(${LIBXML2_LIBRARIES})

# Create and install package configuration and version files.
configure_file(${sys-sage_SOURCE_DIR}/pkg/sys-sage-config.cmake.in
                ${sys-sage_BINARY_DIR}/pkg/sys-sage-config.cmake @ONLY)
configure_file(${sys-sage_SOURCE_DIR}/pkg/sys-sage-config-version.cmake.in
                ${sys-sage_BINARY_DIR}/pkg/sys-sage-config-version.cmake @ONLY)
configure_file(${sys-sage_SOURCE_DIR}/pkg/sys-sage.pc.in
                ${sys-sage_BINARY_DIR}/pkg/sys-sage.pc @ONLY)

install(FILES ${sys-sage_BINARY_DIR}/pkg/sys-sage.pc
         DESTINATION lib)
install(FILES ${sys-sage_BINARY_DIR}/pkg/sys-sage-config.cmake
                ${sys-sage_BINARY_DIR}/pkg/sys-sage-config-version.cmake
         DESTINATION lib/cmake/sys-sage)


###Options:
option(CAT_AWARE "Build and install functionality regarding Intel L3 CAT" OFF)
option(CPUINFO "Build and install functionality regarding Linux cpuinfo" ON)
option(DATA_SOURCES "Build and install all data sources" OFF)
option(DS_HWLOC "Build and install data source hwloc (Retrieves hwloc topology information)" OFF)
option(DS_MT4G "Build and install data source mt4g (Compute and memory topology of NVidia GPUs)" OFF)
option(DS_NUMA "Build and install data source caps-numa-benchmark" OFF)

if(CAT_AWARE)
    add_definitions(-DCAT_AWARE )
endif()

if(DATA_SOURCES)
    set(DS_HWLOC ON)
    set(DS_MT4G ON)
    set(DS_NUMA ON)
endif()

# Configure file based on options
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/topology.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/src/topology.hpp)

# Top-level build just includes subdirectories.
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(data-sources)

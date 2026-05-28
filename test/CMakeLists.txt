cmake_minimum_required(VERSION 3.22)

project(test LANGUAGES CXX)

option(TEST_ASAN "Build tests with enabled address sanitizers" OFF)
option(TEST_TSAN "Build tests with enabled thread sanitizers" OFF)
option(TEST_UBSAN "Build tests with enabled undefined behaviour sanitizers" OFF)
option(TEST_COVERAGE "Build tests with enabled coverage" OFF)

# remove XML tests for now
#add_executable(test test.cpp topology.cpp datapath.cpp hwloc.cpp mt4g.cpp caps-numa-benchmark.cpp proc_cpuinfo.cpp export.cpp import.cpp relation.cpp json.cpp)
add_executable(test test.cpp topology.cpp datapath.cpp hwloc.cpp mt4g.cpp caps-numa-benchmark.cpp proc_cpuinfo.cpp relation.cpp json.cpp)

target_compile_definitions(test PRIVATE SYS_SAGE_TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

find_package(sys-sage REQUIRED)
target_link_libraries(test PRIVATE sys-sage::sys-sage)

add_subdirectory(ut)
target_link_libraries(test PRIVATE ut)

if(${TEST_ASAN})
    target_compile_options(test PRIVATE -fsanitize=address -O0 -g3)
    target_link_options(test PRIVATE -fsanitize=address -O0)
endif()
if(${TEST_TSAN})
    target_compile_options(test PRIVATE -fsanitize=thread -O0 -g3)
    target_link_options(test PRIVATE -fsanitize=thread -O0)
endif()
if(${TEST_UBSAN})
    target_compile_options(test PRIVATE -fsanitize=undefined -O0 -g3)
    target_link_options(test PRIVATE -fsanitize=undefined -O0)
endif()

if(${TEST_COVERAGE})
    target_compile_options(sys-sage PRIVATE --coverage -O0 -g3)
    target_link_options(sys-sage PRIVATE --coverage -O0 -g3)
    # configure_file(coverage.sh.in ${CMAKE_CURRENT_SOURCE_DIR}/coverage.sh @ONLY)
endif()

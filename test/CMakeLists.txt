include_directories(../src) # The include path is not set in the sys-sage target because CMAKE_INCLUDE_CURRENT_DIR is used instead
include_directories(../external_interfaces)

add_subdirectory(ut)
add_executable(test test.cpp topology.cpp datapath.cpp hwloc.cpp mt4g.cpp caps-numa-benchmark.cpp proc_cpuinfo.cpp export.cpp)
target_link_libraries(test PRIVATE ut sys-sage)
target_compile_definitions(test PRIVATE SYS_SAGE_TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

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


link_libraries(sys-sage sys-sage)
include_directories(../src)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(basic_usage basic_usage.cpp)
add_executable(custom_attributes custom_attributes.cpp)
add_executable(gpu-topo-parser gpu-topo-parser.cpp)
add_executable(larger_topo larger_topo.cpp)
add_executable(sys-sage-benchmarking sys-sage-benchmarking.cpp)
add_executable(use_custom_parser custom_parser_musa/use_custom_parser.cpp custom_parser_musa/musa_parser.cpp custom_parser_musa/musa_parser.hpp)

install(TARGETS basic_usage gpu-topo-parser custom_attributes larger_topo sys-sage-benchmarking use_custom_parser DESTINATION bin/examples)
install(DIRECTORY example_data DESTINATION bin/examples)

if(CAT_AWARE)
    add_executable(matmul matmul.cpp)
    set_source_files_properties(matmul.cpp PROPERTIES COMPILE_FLAGS -O3)
    target_link_libraries(matmul hwloc pqos )
    install(TARGETS matmul DESTINATION bin/examples)
endif()

if(CPUINFO)
    add_executable(cpu-frequency cpu-frequency.cpp)
    install(TARGETS cpu-frequency DESTINATION bin/examples)
endif()

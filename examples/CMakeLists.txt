
link_libraries(sys-sage sys-sage)
include_directories(../src)
# include_directories(../src/external_interfaces)
# include_directories(../src/pa)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(basic_usage basic_usage.cpp)
add_executable(custom_attributes custom_attributes.cpp)
add_executable(mt4g-parser mt4g-parser.cpp)
add_executable(larger_topo larger_topo.cpp)
add_executable(sys-sage-benchmarking sys-sage-benchmarking.cpp)
add_executable(use_custom_parser custom_parser_musa/use_custom_parser.cpp custom_parser_musa/musa_parser.cpp custom_parser_musa/musa_parser.hpp)
add_executable(cccbenchplushwloc cccbenchplushwloc.cpp)
add_executable(iqm-test iqm-test.cpp)
add_executable(xml_import xml_import.cpp)
if (QDMI)
    add_executable(qdmi-test qdmi-test.cpp)
endif()

if(PAPI_METRICS)
    add_executable(papi_migrate papi_migrate.cpp)
    #add_executable(papi_basics papi_basics.cpp)
    #add_executable(papi_attach_cpu papi_attach_cpu.cpp)
    #add_executable(papi_attach_process papi_attach_process.cpp)
endif()

install(TARGETS basic_usage mt4g-parser custom_attributes larger_topo sys-sage-benchmarking use_custom_parser cccbenchplushwloc xml_import iqm-test DESTINATION bin/examples)
install(DIRECTORY example_data DESTINATION bin/examples)

if(INTEL_PQOS)
    add_executable(matmul matmul.cpp)
    set_source_files_properties(matmul.cpp PROPERTIES COMPILE_FLAGS -O3)
    target_link_libraries(matmul hwloc pqos )
    install(TARGETS matmul DESTINATION bin/examples)
endif()

if(PROC_CPUINFO)
    add_executable(cpu-frequency cpu-frequency.cpp)
    install(TARGETS cpu-frequency DESTINATION bin/examples)
endif()

if(NVIDIA_MIG)
    add_executable(nvidia-mig nvidia-mig.cpp)
    install(TARGETS nvidia-mig DESTINATION bin/examples)
endif()

if (QDMI)
    target_link_libraries(qdmi-test PRIVATE
      qdmi
      qinfo
      )
endif()

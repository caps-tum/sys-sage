cmake_minimum_required(VERSION 3.22)

project(examples LANGUAGES CXX)

find_package(sys-sage REQUIRED)

add_executable(basic_usage basic_usage.cpp)
target_compile_definitions(basic_usage PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(basic_usage PRIVATE sys-sage::sys-sage)

add_executable(custom_attributes custom_attributes.cpp)
target_compile_definitions(custom_attributes PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(custom_attributes PRIVATE sys-sage::sys-sage)

add_executable(mt4g-parser mt4g-parser.cpp)
target_compile_definitions(mt4g-parser PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(mt4g-parser PRIVATE sys-sage::sys-sage)

add_executable(larger_topo larger_topo.cpp)
target_compile_definitions(larger_topo PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(larger_topo PRIVATE sys-sage::sys-sage)

add_executable(sys-sage-benchmarking sys-sage-benchmarking.cpp)
target_compile_definitions(sys-sage-benchmarking PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(sys-sage-benchmarking PRIVATE sys-sage::sys-sage)

add_executable(use_custom_parser custom_parser_musa/use_custom_parser.cpp custom_parser_musa/musa_parser.cpp custom_parser_musa/musa_parser.hpp)
target_compile_definitions(use_custom_parser PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(use_custom_parser PRIVATE sys-sage::sys-sage)

add_executable(cccbenchplushwloc cccbenchplushwloc.cpp)
target_link_libraries(cccbenchplushwloc PRIVATE sys-sage::sys-sage)

add_executable(iqm-test iqm-test.cpp)
target_link_libraries(iqm-test PRIVATE sys-sage::sys-sage)

add_executable(xml_import xml_import.cpp)
target_compile_definitions(xml_import PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
target_link_libraries(xml_import PRIVATE sys-sage::sys-sage)

if(sys-sage-INTEL_PQOS)
    add_executable(matmul matmul.cpp)
    target_compile_options(matmul PRIVATE -O3)
    target_link_libraries(matmul PRIVATE
        sys-sage::sys-sage
        hwloc
        pqos
    )
endif()

if(sys-sage-NVIDIA_MIG)
    add_executable(nvidia-mig nvidia-mig.cpp)
    target_compile_definitions(nvidia-mig PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
    target_link_libraries(nvidia-mig PRIVATE sys-sage::sys-sage)
endif()

if(sys-sage-PROC_CPUINFO)
    add_executable(cpu-frequency cpu-frequency.cpp)
    target_compile_definitions(cpu-frequency PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
    target_link_libraries(cpu-frequency PRIVATE sys-sage::sys-sage)
endif()

if(sys-sage-PAPI)
    add_executable(papi_basics papi_basics.cpp)
    target_compile_definitions(papi_basics PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
    target_link_libraries(papi_basics PRIVATE sys-sage::sys-sage)

    add_executable(papi_multithreading papi_multithreading.cpp)
    target_compile_definitions(papi_multithreading PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
    target_link_libraries(papi_multithreading PRIVATE sys-sage::sys-sage)

    add_executable(papi_monitor_process papi_monitor_process.cpp)
    target_link_libraries(papi_monitor_process PRIVATE sys-sage::sys-sage)

    add_executable(papi_migrate_cpus papi_migrate_cpus.cpp)
    target_compile_definitions(papi_migrate_cpus PRIVATE EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example_data")
    target_link_libraries(papi_migrate_cpus PRIVATE sys-sage::sys-sage)
endif()

if(sys-sage-PAPI AND sys-sage-PROC_CPUINFO)
    add_executable(papi_greenscore papi_greenscore.cpp)
    target_link_libraries(papi_greenscore PRIVATE sys-sage::sys-sage)
endif()

if(sys-sage-QDMI)
    add_executable(qdmi-test qdmi-test.cpp)
    target_link_libraries(qdmi-test PRIVATE
        sys-sage::sys-sage
        qdmi
        qinfo
    )
endif()

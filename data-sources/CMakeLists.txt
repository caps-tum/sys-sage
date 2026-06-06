###### hwloc
if(DS_HWLOC)
    message(STATUS "DS_HWLOC - hwloc data source")

    add_executable(hwloc-output hwloc-output.cpp)

    target_include_directories(hwloc-output PRIVATE ${HWLOC_INCLUDE_DIRS})
    target_link_directories(hwloc-output PRIVATE ${HWLOC_LIBRARY_DIRS})
    target_link_libraries(hwloc-output PRIVATE ${HWLOC_LIBRARIES})

    find_package(OpenMP COMPONENTS CXX)
    if(OpenMP_FOUND)
        target_link_libraries(hwloc-output PRIVATE OpenMP::OpenMP_CXX)
    endif()

    install(TARGETS hwloc-output DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

###### numa
if(DS_NUMA)
    message(STATUS "DS_NUMA - caps-numa-benchmark data source")

    add_executable(caps-numa-benchmark caps-numa-benchmark.cpp)

    target_include_directories(caps-numa-benchmark PRIVATE ${NUMACTL_INCLUDE_DIRS})
    target_link_directories(caps-numa-benchmark PRIVATE ${NUMACTL_LIB_DIR})
    target_link_libraries(caps-numa-benchmark PRIVATE ${NUMACTL_LIBRARIES})

    install(TARGETS caps-numa-benchmark DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

###### mt4g
if(DS_MT4G)
    message(STATUS "DS_MT4G - mt4g data source")

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mt4g/CMakeLists.txt")
        find_package(Git QUIET)
        if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
            execute_process(
                COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT
            )

            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(FATAL_ERROR "'git submodule update --init --recursive' failed with exit code '${GIT_SUBMOD_RESULT}'")
            endif()
        endif()
    endif()

    add_subdirectory(mt4g)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(SOURCES
    Topology.cpp
    DataPath.cpp
    CAT_aware.cpp
    cpuinfo.cpp
    xml_dump.cpp
    parsers/hwloc.cpp
    parsers/caps-numa-benchmark.cpp
    parsers/gpu-topo.cpp)

set(HEADERS
    sys-sage.hpp
    Topology.hpp
    DataPath.hpp
    xml_dump.hpp
    parsers/hwloc.hpp
    parsers/caps-numa-benchmark.hpp
    parsers/gpu-topo.hpp)

add_library(sys-sage SHARED ${SOURCES} ${HEADERS})

set_target_properties(sys-sage PROPERTIES
#    PUBLIC_HEADER "sys-sage.hpp;Topology.hpp;DataPath.hpp;xml_dump.hpp;parsers/hwloc.hpp;parsers/caps-numa-benchmark.hpp;parsers/gpu-topo.hpp"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

target_include_directories(sys-sage PRIVATE .)

install(DIRECTORY "."
    DESTINATION inc
    FILES_MATCHING PATTERN "*.hpp")
install(TARGETS sys-sage
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION inc
    ARCHIVE DESTINATION lib)

#for spack
install(DIRECTORY "."
    DESTINATION lib/cmake/inc
    FILES_MATCHING PATTERN "*.hpp")
install(TARGETS sys-sage
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION inc
    ARCHIVE DESTINATION lib/cmake/lib)

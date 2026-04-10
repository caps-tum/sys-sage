set(PYBIND11_FINDPYTHON ON)
pybind11_add_module(sys_sage MODULE bindings.cpp)
target_link_libraries(sys_sage PRIVATE pybind11::module)
target_link_libraries(sys_sage PRIVATE sys-sage)

set_target_properties(sys_sage PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS sys_sage DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")

cmake_minimum_required(VERSION 3.22)

project(py_sys_sage VERSION 1.0.0 LANGUAGES CXX)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

find_package(sys-sage REQUIRED)

pybind11_add_module(_py_sys_sage MODULE bindings.cpp)

target_link_libraries(_py_sys_sage PRIVATE sys_sage::sys_sage)

install(TARGETS _py_sys_sage
    LIBRARY DESTINATION py_sys_sage
)

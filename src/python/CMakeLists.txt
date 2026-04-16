set(PYBIND11_FINDPYTHON ON)
pybind11_add_module(sys_sage MODULE bindings.cpp)
target_link_libraries(sys_sage PRIVATE pybind11::module)
target_link_libraries(sys_sage PRIVATE sys-sage)

execute_process(
    COMMAND ${Python_EXECUTABLE} -c "import os, sysconfig; print(os.path.relpath(sysconfig.get_path('purelib'), sysconfig.get_config_var('prefix')))"
    OUTPUT_VARIABLE PYTHON_REL_SITE_PACKAGE_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

install(TARGETS sys_sage DESTINATION "${PYTHON_REL_SITE_PACKAGE_PATH}")

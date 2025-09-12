# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPAPI
-------

Finds the PAPI library.

Result Variables
^^^^^^^^^^^^^^^^

``PAPI_FOUND``
  True if the system has the PAPI library.
``PAPI_INCLUDE_DIRS``
  Include directories needed to use PAPI.
``PAPI_LIBRARIES``
  Libraries needed to link to PAPI.

Cache Variables
^^^^^^^^^^^^^^^

``PAPI_INCLUDE_DIR``
  The directory containing ``papi.h``.
``PAPI_LIBRARY``
  The path to the PAPI library.

#]=======================================================================]

find_path(PAPI_ROOT_DIR
    NAMES include/papi.h
    HINTS ENV PAPI_INSTALL_DIR
)

find_library(PAPI_LIBRARY
    NAMES papi
    HINTS ${PAPI_ROOT_DIR}/lib
)

find_path(PAPI_INCLUDE_DIR
    NAMES papi.h
    HINTS ${PAPI_ROOT_DIR}/include
    PATH_SUFFIXES include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAPI DEFAULT_MSG
    PAPI_LIBRARY
    PAPI_INCLUDE_DIR
)

if(PAPI_FOUND)
  set(PAPI_LIBRARIES ${PAPI_LIBRARY})
  set(PAPI_INCLUDE_DIRS ${PAPI_INCLUDE_DIR})
endif()

mark_as_advanced(
    PAPI_LIBRARIES
    PAPI_INCLUDE_DIRS
)

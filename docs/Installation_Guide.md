# Installation Guide

## C++ API

The C++ library can either be installed via the [Spack](https://spack.readthedocs.io/en/latest/) package manager or directly from the sources with CMake.

### Installation with Spack

Use

```bash
spack info sys-sage
```

to retrieve information about supported options and versions.

To install the library, run

```bash
spack install sys-sage
```

### Installation with CMake

#### Fundamental Dependencies

- gcc (12.1+) or clang (16.0.0+)
- cmake (3.22+)
- libxml2 (2.9.13+)
- nlohmann-json (3.10+)

#### Build option-specific dependencies

- cuda (10+, only when building with the **NVIDIA_MIG** option)
- libpqos (11+, only when building with the **INTEL_PQOS** option)
- numactl (only when building with the **DS_NUMA** option)
- hwloc (2.9+, only when building with the **DS_HWLOC** option)
- papi (only when building with the **PAPI** option)
- backend_ibm, qinfo, qdmi (only when building with the **QDMI** option)

#### Build options

Set the following options to enable features for extended functionality:

<center>
    | CMake option | description |
    | ------------ | ----------- |
    | INTEL_PQOS | builds with Intel CAT support |
    | NVIDIA_MIG | builds with NVIDIA MIG support |
    | PROC_CPUINFO | builds with Linux /proc/cpuinfo support (only x86) |
    | QDMI | builds with QDMI support |
    | PAPI | builds with PAPI support |
    | DATA_SOURCES | enables all data source options |
    | DS_HWLOC | builds with the hwloc data source for automatic CPU topology discovery (independant from hwloc input parser) |
    | DS_MT4g | builds with the mt4g data source for automatic GPU topology discovery (independant from mt4g input parser) |
    | DS_NUMA | builds with the caps-numa-benchmark data source for automatic NUMA topology discovery (independant from caps-numa-benchmark input parser) |
</center>

#### Building

To build and install _sys-sage_ system-wide, run

```bash
git clone https://github.com/caps-tum/sys-sage.git
cd sys-sage
mkdir build && cd build
cmake ..
make -j $(nproc)
sudo make install
```

This will install _sys-sage_ to `/usr/local` on UNIX platforms (see [here](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html)).
If you instead decide to install _sys-sage_ locally on your system, you need to manually set the `LD_LIBRARY_PATH` environment variable and optionally the `CMAKE_PREFIX_PATH` and `PKG_CONFIG_PATH` (or whatever the equivalent is on your platform) if you want to find _sys-sage_ through CMake or pkg-config respectively.

### Packaging

Regardless of the installation method, other projects can use _sys-sage_ in CMake through

```cmake
find_package(sys-sage REQUIRED)
target_link_libraries(<target> {INTERFACE|PUBLIC|PRIVATE} sys-sage::sys-sage)
```

Alternatively, _sys-sage_'s include and library paths can manually be retrieved from pkg-config through

```bash
pkg-config --cflags sys-sage
pkg-config --libs sys-sage
```

## Python API

_sys-sage_ provides additional bindings for the Python programming language through the _py_sys_sage_ package.
The bindings depend on the C++ library, so first install the C++ library as described in the above [section](#c-api) and make sure that the `CMAKE_PREFIX_PATH` is set accordingly if installed locally with CMake.

### Installation with Pip

#### Additional Dependencies

- pybind11
- pybind11-json
- pip

To manage your packages locally, you can create an isolated virtual environment by running

```bash
python3 -m venv <path_to_venv>
```

You can also reuse an existing virtual environment or install it globally on your system depending on your platform.
In the following, we'll proceed with the installation into a virtual environment, but the steps are almost identical in other cases.

From the project root directory of _sys-sage_, run

```bash
<path_to_venv>/bin/python -m pip install ./python
```

to install the package.

### Package Import

The package is called `py_sys_sage` and it can be imported like this:

```Python
import py_sys_sage as pysage
```



# sys-sage

<img src="docs/images/logo-white.jpeg" alt="sys-sage logo" style="float: right;" width="200"/>

**sys-sage** is a C++ library and toolkit for capturing, representing, and analyzing the hardware topology of compute systems.
It provides a unified, extensible interface to query, store, and manipulate hardware information from diverse sources, supporting both classical HPC and emerging quantum computing environments.

## Overview

**sys-sage** aims to provide a unified abstraction for hardware topology, making it easier to develop portable and extensible tools for system introspection, resource management, and performance analysis. It is designed for both traditional HPC and quantum-accelerated systems, without making a strict distinction between the two.

### Features

- **Unified Hardware Topology Representation:** Abstracts CPUs, GPUs, QPUs, accelerators, memory, and more.
- **Multi-Source Data Integration:** Gathers and merges data from various sources -- some of them already available out-of-the-box (e.g., hwloc), others can easily be created by the user using the API.
- **Extensible Parser System:** Easily add new data sources or custom parsers.
- **Rich C++ API:** Query, traverse, and manipulate system topology programmatically.
- **Support for Classical HPC and HPCQC:** Designed for HPC, extensible to quantum and hybrid systems.
<!-- - **Comprehensive Documentation:** Includes guides, API reference, and practical examples. -->


## Documentation

The documentation is available [here](https://stepanvanecek.github.io/sys-sage/latest/html/index.html).

<!-- - [Installation Guide](docs/Installation_Guide.md)
- [Concepts & Architecture](docs/Concept.md)
- [Usage Examples](docs/Usage.md)
- [Data Parsers](docs/Data_Parsers.md)
- [API Reference (Doxygen)](docs/html/index.html) -->

## Usage

```cpp
#include <iostream>

#include "sys-sage.hpp"

int main(int argc, char *argv[])
{
    sys_sage::Node* n = new sys_sage::Node(1);
    sys_sage::parseHwlocOutput(n, <hwloc_xml_dump_path>);
    std::cout << "Total num HW threads: " << n->CountAllSubcomponentsByType(sys_sage::ComponentType::Thread) << std::endl;

    std::cout << "---------------- Printing the whole tree ----------------" << std::endl;
    n->PrintSubtree();

    n->Delete(true);
    return 0;
}
```

Refer to `examples/` for more showcases of how to use sys-sage.

## Installation

Please refer to the [Installation Guide](https://stepanvanecek.github.io/sys-sage/html/md__installation__guide.html) for more information.

The recommended installation is through spack. Alternatively, sys-sage can be built from the sources.

### Installation with spack
```bash
spack install sys-sage
#see "spack info sys-sage" for available options and versions
```

### Installation with CMAKE

#### Basic dependencies

- cmake (3.22+)
- libxml2 (2.9.13+)
- nlohmann-json (3.11+)

#### Build option-specific dependencies

- cuda (11+, only when building with the **NVIDIA_MIG** option)
- libpqos (11+, only when building with the **INTEL_CAT** option)
- numactl (only when building **caps-numa-benchmark data source**)
- hwloc (2.9+, only when building **hwloc data source**)

#### Building from sources

```bash
git clone https://github.com/caps-tum/sys-sage.git
cd sys-sage
mkdir build && cd build
cmake ..
# build options:
# -DINTEL_PQOS=ON           - builds with Intel CAT functionality. For that, Intel-specific pqos header/library are necessary.
# -DNVIDIA_MIG=ON           - Build and install functionality regarding NVidia MIG(multi-instance GPU, ampere or newer).
# -DPROC_CPUINFO=ON         - Build and install functionality regarding Linux cpuinfo (only x86) -- default ON.
# -DDATA_SOURCES=ON         - builds all data sources from folder 'data-sources' listed below. Data sources are used to collecting HW-related information, so it only makes sense to compile that on the system where the topology information is queried.
# -DDS_HWLOC=ON             - builds the hwloc data source for retrieving the CPU topology
# -DDS_MT4g=ON              - builds the mt4g data source for retrieving GPU compute and memory topology. If turned on, includes hwloc.
# -DDS_NUMA=ON              - builds the caps-numa-benchmark. If turned on, includes Linux-specific libraries.
# -DPAPI=ON                 - builds with PAPI support. If turned on, includes PAPI library headers.
# -DCMAKE_INSTALL_PREFIX=../inst-dir    - to install locally into the git repo folder
make all install
```

## Testing

Please refer to the [Testing Guide](test/README.md) for more information.

## About

sys-sage has been created by Stepan Vanecek (stepan.vanecek@tum.de) and the [CAPS TUM](https://www.ce.cit.tum.de/en/caps/homepage/). Please contact us in case of questions, bug reporting etc.

sys-sage is available under the Apache-2.0 license. (see [License](https://github.com/caps-tum/sys-sage/blob/master/LICENSE))

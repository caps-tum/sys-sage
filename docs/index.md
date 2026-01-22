# sys-sage: A Library for Representing System Architecture and Dynamic Properties

**sys-sage** is a modern, extensible C++ library (with a **C++ and Python APIs**) for describing, analyzing, and manipulating system architectures and their dynamic properties. Whether you’re working with classic HPC clusters, heterogeneous nodes, or exploring new computing paradigms, sys-sage gives you a unified, flexible way to represent hardware, interconnections, static or dynamic system state, capabilities, properties, and settings information, momentual measurements, or application-specific hardware-relevant information, and all the metadata that matters.


### What is sys-sage?

At its core, sys-sage is a toolkit for building, managing, and querying a **rich, extensible model of your system’s architecture**. It’s designed for:

- **Researchers** who want to experiment with new hardware or system layouts and who need to correlate system information from differnt sources to get a more comprehensive understaiding of a system.
- **Tool developers**  building schedulers, mappers, or simulators.
- **System architects** who need to capture and reason about complex, evolving topologies.

sys-sage models your system as a connection of a **component tree** (nodes, chips, cores, memory, accelerators, etc.) and a **relations graph** (data paths, logical connections, and more). You build a hierarchical, easy-to-understand system representation using the component tree, and you can model (physical or logical) relations or interactions within the system using the relations graph -- providing both an easy-to-understand and a powerful and flexible building blocks. You can attach arbitrary attributes to any component or relation, making it easy to store calibration data, performance counters, or anything else your specific use-case cares about.

While sys-sage is deeply rooted in HPC and classical hardware, it’s built to be future-proof: you can naturally represent new types of resources (including GPUs, quantum devices, or custom accelerators) and their relationships, **all in the same model**.

### Why use sys-sage?

- **Unified model:** Represent all your hardware and logical resources in one place.
- **Extensible:** Add new 3rd-party data sources or APIs, or attach custom metadata, without changing the core.
- **Easy traversal and queries:** Find all GPUs, all data paths, or all components with a certain property.
- **Serialization:** Import/export your system topology (with all metadata) to XML, with hooks for custom attributes -- and recreate the exact system representation, including all internal information, on a different machine or at a different time.
- **Python API:** Use sys-sage from Python for rapid prototyping, data science, or integration with other tools.

### Where sys-sage Shines

- **Heterogeneous systems:** Model CPUs, GPUs, quantum devices, and their interconnects in one place.
- **Custom metadata:** Attach anything you need — calibration data, performance counters, logical mappings, etc.
- **Extensible and future-proof:** Add new tool/interface integrations, topologies, and relations as hardware evolves.
- **Open and hackable:** Source code available to make understanding and custom extensions easy.


## Documentation overview
- [Detailed Concepts and Architecture](Concept.md)
- [Installation Guide](Installation_Guide.md)
- [Data Parsers](Data_Parsers.md)
- [PAPI Integration](sys-sage_PAPI.md)
- [**Python API**](python-sys-sage.md)

- **API documentation**
    - [Component](class_component.html)
        - [Topology](class_topology.html)
        - [Node](class_node.html)
        - [Memory](class_memory.html)
        - [Storage](class_storage.html)
        - [Chip](class_chip.html)
        - [Cache](class_cache.html)
        - [Subdivision](class_subdivision.html)
        - [Numa](class_numa.html)
        - [Core](class_core.html)
        - [Thread](class_thread.html)
        - [QuantumBackend](class_quantumbackend.html)
        - [Qubit](class_qubit.html)
        - [AtomSite](class_atomsite.html)
    - [Relation](class_relation.html)
        - [DataPath](class_datapath.html)
        - [QuantumGate](class_quantumgate.html)
        - [CouplingMap](class_couplingmap.html)
    - External Interfaces
        - Intel PQOS
        - NVidia MIG
        - /proc/cpuinfo
    - Input Parsers
        - caps-numa-benchmark
        - cccbench
        - [**hwloc**](hwloc_8hpp.html)
        - iqm
        - mt4g
        - qdmi
    - Data Sources
        - caps-numa-benchmark
        - [**hwloc**](hwloc-output_8cpp.html)
        - mt4g
    - [XML import/export](xmlio.md)
    <!-- TODO: is XML import/export API or concept? -->
    


## Python API

sys-sage now offers a Python API for rapid prototyping and integration with Python-based tools. You can:

- Load and traverse system topologies
- Query components and relations
- Export/import XML
- Attach and retrieve custom attributes

See [sys-sage Python API doc](python-sys-sage.md) for details and usage examples.

## Examples: 

#### Building and Exporting a System Topology

```cpp
#include <sys_sage.hpp>

// Create a node with a CPU and a GPU
auto* node = new sys_sage::Node(0, "Node0");
auto* cpu = new sys_sage::Chip(node, 1, "CPU0");
auto* gpu = new sys_sage::Chip(node, 2, "GPU0");

// Connect CPU and GPU with a DataPath 
auto* dp = new sys_sage::DataPath(cpu, gpu, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathType::Physical);
dp->SetBandwidth(16.0); // GB/s
dp->SetLatency(1.2);    // microseconds

// Export to XML
sys_sage::exportToXml(node, "system.xml");
```

#### Querying Components and Relations

```cpp
// Find all "chips" in the system
std::vector<sys_sage::Component*> chips;
node->GetAllSubcomponentsByType(sys_sage::ComponentType::Chip, &chips);

// Print all DataPaths from CPU to GPU
for (auto* rel : cpu->GetAllRelationsBy(sys_sage::RelationType::DataPath)) {
    rel->Print();
}
```


#### Python API

```python
import sys_sage

root = sys_sage.import_from_xml("system.xml")
for gpu in root.get_all_subcomponents_by_type("Gpu"):
    print(gpu.name, gpu.get_attribute("bandwidth"))
```

### Documentation Versioning

- [latest](https://stepanvanecek.github.io/sys-sage/latest/html/index.html)
- [1.0.0](https://stepanvanecek.github.io/sys-sage/1.0.0/html/index.html)
- [0.5.2](https://stepanvanecek.github.io/sys-sage/0.5.2/html/index.html)

## About

sys-sage has been created by Stepan Vanecek (stepan.vanecek@tum.de) and the [CAPS TUM](https://www.ce.cit.tum.de/en/caps/homepage/). Please contact us in case of questions, bug reporting etc.

The source code can be found at [https://github.com/caps-tum/sys-sage](https://github.com/caps-tum/sys-sage) .

sys-sage is available under the Apache-2.0 license. (see [License](https://github.com/caps-tum/sys-sage/blob/master/LICENSE))

Version: 1.0.0
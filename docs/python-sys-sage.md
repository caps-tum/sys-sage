# sys-sage Python API documentation

The _sys-sage_ library provides bindings for the Python programming language through the _py_sys_sage_ package.
The bindings are built using the [pybind11](https://pybind11.readthedocs.io/en/stable/basics.html) library.
You may find further information on how to use Python bindigs in general there.

## Installation

Information on how to install the bindings can be found [here](Installation_Guide.md#python-api).

## Usage

Since the bindings simply call the underlying C++ functions, using _sys-sage_'s Python API is akin to using _sys-sage_'s C++ API.
We have marked all functions and class methods that have an equivalent Python binding with the "Python" tag.
They can be found in their respective API documentation (e.g. see sys_sage::Component::InsertChild).

Member variables that are public or have getters and setters in the C++ API can simply be accessed as properties of the Python object.

The code snippet below gives a small example:

```Python
import py_sys_sage as pysage

component = pysage.Component(1, "MyComponent")
print(f"{component.name} is of type {component.GetComponentTypeStr()}")

component.SetAttribute("myInteger", 10)
print(f"{component.name} has an attribute named 'myInteger' with value {component.GetAttribute("myInteger")}")

node = pysage.Node()
pysage.parseHwlocOutput(node, "topo.xml")
```

Some C++ API calls are enveloped inside of a thin wrapper for the bindings, resulting in slightly different function signatures for the Python API.
Furthermore, some API calls are specific to the Python bindings and have no direct equivalent to the C++ API.
In the following we list these API calls and show how they can be used in Python:

### CalcSubtreeSize

C++ API: sys_sage::Component::CalcSubtreeSize.

Python API:

```Python
component_size, RelationSize = component.CalcSubtreeSize()
```

### IterateAttributes

C++ API: No equivalent. Python-specific method used to iterate over attributes of components or relations.

Python API:

```Python
for key, value in component.IterateAttributes():
    print(key, value)
```

### DumpJson

C++ API: sys_sage::DumpJson().

Python API:

```Python
jsonObject = pysage.DumpJson(component)
```

### PAPI integrations

C++ API: sys_sage::SS_PAPI_start, sys_sage::SS_PAPI_read, sys_sage::SS_PAPI_accum, sys_sage::SS_PAPI_stop

Python API:

```Python
# If no prior metricsRelation exits, pass `None` to the function
(returnValue, newMetricsRelation) = pysage.SS_PAPI_start(eventSet, metricsRelation)
metricsRelation = newMetricsRelation # newMetricsRelation might be the same as metricsRelation

returnValue, timeStamp = pysage.SS_PAPI_read(metricsRelation, componentRoot, permanentBool)

returnValue, timeStamp = pysage.SS_PAPI_accum(metricsRelation, componentRoot, permanentBool)

returnValue, timeStamp = pysage.SS_PAPI_stop(metricsRelation, componentRoot, permanentBool)
```

### Constants

The following shows a mapping of _sys-sage_'s C++ API constants to the equivalent Python API constants.

<center>
    |                C++                 |                Python              |
    | ---------------------------------- | ---------------------------------- |
    | sys_sage::ComponentType::Any                | pysage.COMPONENT_ANY                     |
    | sys_sage::ComponentType::Generic                | pysage.COMPONENT_GENERIC                     |
    | sys_sage::ComponentType::Thread              | pysage.COMPONENT_THREAD                   |
    | sys_sage::ComponentType::Core                | pysage.COMPONENT_CORE                     |
    | sys_sage::ComponentType::Cache               | pysage.COMPONENT_CACHE                    |
    | sys_sage::ComponentType::Subdivision         | pysage.COMPONENT_SUBDIVISION              |
    | sys_sage::ComponentType::Numa                | pysage.COMPONENT_NUMA                     |
    | sys_sage::ComponentType::Chip                | pysage.COMPONENT_CHIP                     |
    | sys_sage::ComponentType::Memory              | pysage.COMPONENT_MEMORY                   |
    | sys_sage::ComponentType::Storage             | pysage.COMPONENT_STORAGE                  |
    | sys_sage::ComponentType::Node                | pysage.COMPONENT_NODE                     |
    | sys_sage::ComponentType::QuantumBackend      | pysage.COMPONENT_QUANTUMBACKEND           |
    | sys_sage::ComponentType::AtomSite            | pysage.COMPONENT_ATOMSITE                 |
    | sys_sage::ComponentType::Qubit               | pysage.COMPONENT_QUBIT                    |
    | sys_sage::ComponentType::Topology            | pysage.COMPONENT_TOPOLOGY                 |
    | sys_sage::SubdivisionCategory::None              | pysage.SUBDIVISION_CATEGORY_NONE              |
    | sys_sage::SubdivisionCategory::GpuSM             | pysage.SUBDIVISION_CATEGORY_GPU_SM            |
    | sys_sage::ChipCategory::None                     | pysage.CHIP_CATEGORY_NONE                     |
    | sys_sage::ChipCategory::Cpu                      | pysage.CHIP_CATEGORY_CPU                      |
    | sys_sage::ChipCategory::CpuSocket                | pysage.CHIP_CATEGORY_CPU_SOCKET               |
    | sys_sage::ChipCategory::Gpu                      | pysage.CHIP_CATEGORY_GPU                      |
    | sys_sage::RelationType::Any                  | pysage.RELATION_TYPE_ANY                  |
    | sys_sage::RelationType::Relation             | pysage.RELATION_TYPE_RELATION             |
    | sys_sage::RelationType::DataPath             | pysage.RELATION_TYPE_DATAPATH             |
    | sys_sage::RelationType::QuantumGate          | pysage.RELATION_TYPE_QUANTUMGATE          |
    | sys_sage::RelationType::CouplingMap          | pysage.RELATION_TYPE_COUPLINGMAP          |
    | sys_sage::RelationCategory::Any          | pysage.RELATION_CATEGORY_ANY          |
    | sys_sage::RelationCategory::Default          | pysage.RELATION_CATEGORY_DEFAULT          |
    | sys_sage::RelationCategory::PAPI_Metrics          | pysage.RELATION_CATEGORY_PAPI_METRICS          |
    | sys_sage::DataPathCategory::Any                  | pysage.DATAPATH_CATEGORY_ANY                  |
    | sys_sage::DataPathCategory::None                 | pysage.DATAPATH_CATEGORY_NONE                 |
    | sys_sage::DataPathCategory::Logical              | pysage.DATAPATH_CATEGORY_LOGICAL              |
    | sys_sage::DataPathCategory::Physical             | pysage.DATAPATH_CATEGORY_PHYSICAL             |
    | sys_sage::DataPathCategory::Datatransfer         | pysage.DATAPATH_CATEGORY_DATATRANSFER         |
    | sys_sage::DataPathCategory::L3CAT                | pysage.DATAPATH_CATEGORY_L3CAT                |
    | sys_sage::DataPathCategory::MIG                  | pysage.DATAPATH_CATEGORY_MIG                  |
    | sys_sage::DataPathCategory::C2C                  | pysage.DATAPATH_CATEGORY_C2C                  |
    | sys_sage::DataPathDirection::Any             | pysage.DATAPATH_DIRECTION_ANY             |
    | sys_sage::DataPathDirection::Outgoing        | pysage.DATAPATH_DIRECTION_OUTGOING        |
    | sys_sage::DataPathDirection::Incoming        | pysage.DATAPATH_DIRECTION_INCOMING        |
    | sys_sage::DataPathOrientation::Oriented      | pysage.DATAPATH_ORIENTATION_ORIENTED      |
    | sys_sage::DataPathOrientation::Bidirectional | pysage.DATAPATH_ORIENTATION_BIDIRECTIONAL |
    | sys_sage::QuantumGateCategory::Unknown           | pysage.QUANTUMGATE_CATEGORY_UNKNOWN           |
    | sys_sage::QuantumGateCategory::Id                | pysage.QUANTUMGATE_CATEGORY_ID                |
    | sys_sage::QuantumGateCategory::X                 | pysage.QUANTUMGATE_CATEGORY_X                 |
    | sys_sage::QuantumGateCategory::Rz                | pysage.QUANTUMGATE_CATEGORY_RZ                |
    | sys_sage::QuantumGateCategory::Cnot              | pysage.QUANTUMGATE_CATEGORY_CNOT              |
    | sys_sage::QuantumGateCategory::Sx                | pysage.QUANTUMGATE_CATEGORY_SX                |
    | sys_sage::QuantumGateCategory::Toffoli           | pysage.QUANTUMGATE_CATEGORY_TOFFOLI           |
</center>

An example would be:

```Python
component.CountAllSubcomponentsByType(pysage.COMPONENT_ANY)
```

### Querying C++ library build configurations

Since _sys-sage_ can be built with different options to enable/disable certain features (see [here](Installation_Guide.md#build-options)), you may want to check if the feature is available through Python.
We provide several module attributes for this purpose:

<center>
    | Module Attributes |
    | ----------------- |
    | pysage.HAS_INTEL_PQOS |
    | pysage.HAS_NVIDIA_MIG |
    | pysage.HAS_PROC_CPUINFO |
    | pysage.HAS_DS_HWLOC |
    | pysage.HAS_DS_MT4G |
    | pysage.HAS_DS_NUMA |
    | pysage.HAS_QDMI |
    | pysage.HAS_PAPI |
</center>

You could for instane do:

```bash
python3 -c "import py_sys_sage as pysage; print(pysage.HAS_PAPI)"
```

## Functional Differences between the C++ and Python API

Some library features within _sys-sage_'s C++ API work slightly different for the Python bindings.
Whenever this is the case, it is always documented explicitly in the documentation sections of the respective library features (e.g. see [here](JSON_Serialization_and_Deserialization.md#python-bindings)).
Please check the documentation.

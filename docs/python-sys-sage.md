# sys-sage Python API documentation

The _sys-sage_ library provides bindings for the Python programming language through the _py_sys_sage_ package. This documenation offers a brief introduction.

## Installation

The bindings depend on the _sys-sage_ library. **First make sure that you have
installed _sys-sage_ and `pybind11` on your system**.

The bindings can be installed through the pip package manager. To manage your
packages locally, you can create an isolated virtual environment by running

```bash
python3 -m venv <path_to_venv>
```

You can also reuse an existing virtual environment or install it globally on
your system depending on your platform. In the following, we'll procede with
the installation into a virtual environment, but the steps are almost identical
in other cases.

From the project root directory of _sys-sage_, run

```bash
<path_to_venv>/bin/python -m pip install ./python
```

to install the package.

## Package Import

The package is called `py_sys_sage` and it can be imported like this:

```Python
import py_sys_sage as pysage
```

## Object initialisation

To create Component and Relation objects, you can utilize the provided initialization functions. For instance, a Component object can be instantiated as follows:

```Python
c = pysage.Component(parent, 0, "Generic Component")
```
The _sys-sage_ library exposes all known constructors for `Component` and its subclasses as Python initialization functions, complete with their default values. In the example above, if the component type is not explicitly provided, it will automatically be assigned ```None``` during initialization.

Throughout this documentation, `c` will consistently refer to a Component object.

## Methods

Calling class methods in Python is very similar to the C++ version:

```Python
c.InsertChild(child)
```
However, certain attributes, such as id and name, are not accessible via standard getters and setters. The specific methods for reading and writing these attributes are detailed in the "Attributes" section of this documentation.

## Module functions

All the default parsers of the _sys-sage_ library are also included in the Python module:

```python
pysage.parseCapsNumaBenchmark(root,"benchmark.csv",";")
```

## Attributes

### Module Attributes

The Python module provides a one-to-one correspondance to the known _sys-sage_ constants.

|                C++                 |                Python              |
| ---------------------------------- | ---------------------------------- |
| ComponentType::None                | COMPONENT_NONE                     |
| ComponentType::Thread              | COMPONENT_THREAD                   |
| ComponentType::Core                | COMPONENT_CORE                     |
| ComponentType::Cache               | COMPONENT_CACHE                    |
| ComponentType::Subdivision         | COMPONENT_SUBDIVISION              |
| ComponentType::Numa                | COMPONENT_NUMA                     |
| ComponentType::Chip                | COMPONENT_CHIP                     |
| ComponentType::Memory              | COMPONENT_MEMORY                   |
| ComponentType::Storage             | COMPONENT_STORAGE                  |
| ComponentType::Node                | COMPONENT_NODE                     |
| ComponentType::QuantumBackend      | COMPONENT_QUANTUMBACKEND           |
| ComponentType::AtomSite            | COMPONENT_ATOMSITE                 |
| ComponentType::Qubit               | COMPONENT_QUBIT                    |
| ComponentType::Topology            | COMPONENT_TOPOLOGY                 |
| SubdivisionCategory::None              | SUBDIVISION_CATEGORY_NONE              |
| SubdivisionCategory::GpuSM             | SUBDIVISION_CATEGORY_GPU_SM            |
| ChipCategory::None                     | CHIP_CATEGORY_NONE                     |
| ChipCategory::Cpu                      | CHIP_CATEGORY_CPU                      |
| ChipCategory::CpuSocket                | CHIP_CATEGORY_CPU_SOCKET               |
| ChipCategory::Gpu                      | CHIP_CATEGORY_GPU                      |
| RelationType::Any                  | RELATION_TYPE_ANY                  |
| RelationType::Relation             | RELATION_TYPE_RELATION             |
| RelationType::DataPath             | RELATION_TYPE_DATAPATH             |
| RelationType::QuantumGate          | RELATION_TYPE_QUANTUMGATE          |
| RelationType::CouplingMap          | RELATION_TYPE_COUPLINGMAP          |
| DataPathCategory::Any                  | DATAPATH_CATEGORY_ANY                  |
| DataPathCategory::None                 | DATAPATH_CATEGORY_NONE                 |
| DataPathCategory::Logical              | DATAPATH_CATEGORY_LOGICAL              |
| DataPathCategory::Physical             | DATAPATH_CATEGORY_PHYSICAL             |
| DataPathCategory::Datatransfer         | DATAPATH_CATEGORY_DATATRANSFER         |
| DataPathCategory::L3CAT                | DATAPATH_CATEGORY_L3CAT                |
| DataPathCategory::MIG                  | DATAPATH_CATEGORY_MIG                  |
| DataPathCategory::C2C                  | DATAPATH_CATEGORY_C2C                  |
| DataPathDirection::Any             | DATAPATH_DIRECTION_ANY             |
| DataPathDirection::Outgoing        | DATAPATH_DIRECTION_OUTGOING        |
| DataPathDirection::Incoming        | DATAPATH_DIRECTION_INCOMING        |
| DataPathOrientation::Oriented      | DATAPATH_ORIENTATION_ORIENTED      |
| DataPathOrientation::Bidirectional | DATAPATH_ORIENTATION_BIDIRECTIONAL |
| QuantumGateCategory::Unknown           | QUANTUMGATE_CATEGORY_UNKNOWN           |
| QuantumGateCategory::Id                | QUANTUMGATE_CATEGORY_ID                |
| QuantumGateCategory::X                 | QUANTUMGATE_CATEGORY_X                 |
| QuantumGateCategory::Rz                | QUANTUMGATE_CATEGORY_RZ                |
| QuantumGateCategory::Cnot              | QUANTUMGATE_CATEGORY_CNOT              |
| QuantumGateCategory::Sx                | QUANTUMGATE_CATEGORY_SX                |
| QuantumGateCategory::Toffoli           | QUANTUMGATE_CATEGORY_TOFFOLI           |

An example would be:

```Python
c.CountAllSubcomponentsByType(pysage.COMPONENT_NONE)
```

### Properties
As previously mentioned, some attributes aren't accessed using traditional getters or setters. Instead, `sys-sage` offers a more pythonic approach to access them:

```Python
name = c.name
c.name = "name"
```

### Dynamic Attributes

Similar to the core _sys-sage_ library, users can define their own custom attributes.
The usage resembles that of dictionaries.

```Python
c["foo"] = "test"
c["foo"]            # returns "test"
c["bar"] = 1        # values can be arbitrary
c[0]                # returns 1, since key-value pairs are ordered in lexicographical order of the keys
```
The same holds for relations.

## XML I/O

XML import and export is enabled, however there are some difference to the original library functions.

### XML Export
When exporting data in _sys-sage_, users can define custom functions to parse component attributes. Two distinct functions can be specified: one for simple attributes and another for complex attributes.

For simple attributes, the custom function must return either a string or None. The processing of the key-value pair is entirely at the user's discretion. Here's an example:

```Python
def search_custom_simple(key: str, value) -> str:
    if "example" in key:
        return str(value)
    return None
```
For complex attributes, the function is expected to return a string representation of an XML node. The encapsulating root element (e.g., <root></root>) should be omitted. For instance:

```Python
def search_custom_complex(key: str, value) -> str:
    # This function returns an XML fragment representing the complex attribute
    return f"<Attribute key=\"{key}\" value=\"{value}\"/>"
```
After defining your custom user functions, the export process can be initiated by calling the pysage.export function, passing the root component, the desired export path, and your custom functions:

```Python
pysage.export(root, path, search_custom_simple, search_custom_complex)
```

### XML Import
The XML import functionality mirrors the export function, but with key differences that are best illustrated through examples.

For simple attributes, all custom attributes from the XML file are passed to the user-provided custom parsing function as XML nodes (strings). Therefore, the custom function must employ string operations to extract the desired value:

```Python
def import_search_simple(x):
    # x will be an XML string like '<Attribute key="benchmark" value="some_value"/>'
    if "benchmark" in x:
        y = x.split("value=\"")[-1]
        return y.rstrip("\"/>")
    return None
```

Similarly, for complex attributes, the XML node as a string is forwarded. However, unlike the simple attribute function, the custom function for complex attributes also gains access to the respective Component object (`c`). This allows for more profound modifications to the component's attributes. This function is expected to return only a success status (e.g., 1 for success, 0 for failure).

```Python
def import_search_complex(x, c):
    # x will be an XML string like '<Attribute key="complex" value="123"/>'
    # c is the Component object to which this attribute belongs
    if "complex" in x:
        y = x.split("value=\"")[-1]
        val = y.rstrip("\"/>")
        c["complex"] = int(val)  # Modify the component's attribute directly
        return 1  # Indicate success
    return 0  # Indicate failure
```

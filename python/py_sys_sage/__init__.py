# Expose the bindings in the internal module library `_py_sys_sage.so` into the
# Python package `py_sys_sage`.
#
# We could simply do `from ._py_sys_sage import *` instead, but this likely
# pollutes the `py_sys_sage` namespace with internally generated functions from
# pybind11. Maybe it doesn't? Anyways, this gives us more control over the
# public API of the bindings.

from . import _py_sys_sage

# Attributes

HAS_INTEL_PQOS = _py_sys_sage.HAS_INTEL_PQOS
HAS_NVIDIA_MIG = _py_sys_sage.HAS_NVIDIA_MIG
HAS_PROC_CPUINFO = _py_sys_sage.HAS_PROC_CPUINFO
HAS_DS_HWLOC = _py_sys_sage.HAS_DS_HWLOC
HAS_DS_MT4G = _py_sys_sage.HAS_DS_MT4G
HAS_DS_NUMA = _py_sys_sage.HAS_DS_NUMA
HAS_QDMI = _py_sys_sage.HAS_QDMI
HAS_PAPI = _py_sys_sage.HAS_PAPI

COMPONENT_NONE = _py_sys_sage.COMPONENT_NONE
COMPONENT_THREAD = _py_sys_sage.COMPONENT_THREAD
COMPONENT_CORE = _py_sys_sage.COMPONENT_CORE
COMPONENT_CACHE = _py_sys_sage.COMPONENT_CACHE
COMPONENT_SUBDIVISION = _py_sys_sage.COMPONENT_SUBDIVISION
COMPONENT_NUMA = _py_sys_sage.COMPONENT_NUMA
COMPONENT_CHIP = _py_sys_sage.COMPONENT_CHIP
COMPONENT_MEMORY = _py_sys_sage.COMPONENT_MEMORY
COMPONENT_STORAGE = _py_sys_sage.COMPONENT_STORAGE
COMPONENT_NODE = _py_sys_sage.COMPONENT_NODE
COMPONENT_QUANTUMBACKEND = _py_sys_sage.COMPONENT_QUANTUMBACKEND
COMPONENT_ATOMSITE = _py_sys_sage.COMPONENT_ATOMSITE
COMPONENT_QUBIT = _py_sys_sage.COMPONENT_QUBIT
COMPONENT_TOPOLOGY = _py_sys_sage.COMPONENT_TOPOLOGY

SUBDIVISION_TYPE_NONE = _py_sys_sage.SUBDIVISION_TYPE_NONE
SUBDIVISION_TYPE_GPU_SM = _py_sys_sage.SUBDIVISION_TYPE_GPU_SM

CHIP_TYPE_NONE = _py_sys_sage.CHIP_TYPE_NONE
CHIP_TYPE_CPU = _py_sys_sage.CHIP_TYPE_CPU
CHIP_TYPE_CPU_SOCKET = _py_sys_sage.CHIP_TYPE_CPU_SOCKET
CHIP_TYPE_GPU = _py_sys_sage.CHIP_TYPE_GPU

RELATION_TYPE_ANY = _py_sys_sage.RELATION_TYPE_ANY
RELATION_TYPE_RELATION = _py_sys_sage.RELATION_TYPE_RELATION
RELATION_TYPE_DATAPATH = _py_sys_sage.RELATION_TYPE_DATAPATH
RELATION_TYPE_QUANTUMGATE = _py_sys_sage.RELATION_TYPE_QUANTUMGATE
RELATION_TYPE_COUPLINGMAP = _py_sys_sage.RELATION_TYPE_COUPLINGMAP

RELATION_CATEGORY_ANY = _py_sys_sage.RELATION_CATEGORY_ANY
RELATION_CATEGORY_DEFAULT = _py_sys_sage.RELATION_CATEGORY_DEFAULT

if _py_sys_sage.HAS_PAPI:
    RELATION_CATEGORY_PAPI_METRICS = _py_sys_sage.RELATION_CATEGORY_PAPI_METRICS

DATAPATH_TYPE_ANY = _py_sys_sage.DATAPATH_TYPE_ANY
DATAPATH_TYPE_NONE = _py_sys_sage.DATAPATH_TYPE_NONE
DATAPATH_TYPE_LOGICAL = _py_sys_sage.DATAPATH_TYPE_LOGICAL
DATAPATH_TYPE_PHYSICAL = _py_sys_sage.DATAPATH_TYPE_PHYSICAL
DATAPATH_TYPE_DATATRANSFER = _py_sys_sage.DATAPATH_TYPE_DATATRANSFER
DATAPATH_TYPE_L3CAT = _py_sys_sage.DATAPATH_TYPE_L3CAT
DATAPATH_TYPE_MIG = _py_sys_sage.DATAPATH_TYPE_MIG
DATAPATH_TYPE_C2C = _py_sys_sage.DATAPATH_TYPE_C2C

DATAPATH_DIRECTION_ANY = _py_sys_sage.DATAPATH_DIRECTION_ANY
DATAPATH_DIRECTION_OUTGOING = _py_sys_sage.DATAPATH_DIRECTION_OUTGOING
DATAPATH_DIRECTION_INCOMING = _py_sys_sage.DATAPATH_DIRECTION_INCOMING

DATAPATH_ORIENTATION_ORIENTED = _py_sys_sage.DATAPATH_ORIENTATION_ORIENTED
DATAPATH_ORIENTATION_BIDIRECTIONAL = _py_sys_sage.DATAPATH_ORIENTATION_BIDIRECTIONAL

QUANTUMGATE_TYPE_UNKNOWN = _py_sys_sage.QUANTUMGATE_TYPE_UNKNOWN
QUANTUMGATE_TYPE_ID = _py_sys_sage.QUANTUMGATE_TYPE_ID
QUANTUMGATE_TYPE_X = _py_sys_sage.QUANTUMGATE_TYPE_X
QUANTUMGATE_TYPE_RZ = _py_sys_sage.QUANTUMGATE_TYPE_RZ
QUANTUMGATE_TYPE_CNOT = _py_sys_sage.QUANTUMGATE_TYPE_CNOT
QUANTUMGATE_TYPE_SX = _py_sys_sage.QUANTUMGATE_TYPE_SX
QUANTUMGATE_TYPE_TOFFOLI = _py_sys_sage.QUANTUMGATE_TYPE_TOFFOLI

# classes

Component = _py_sys_sage.Component
Topology = _py_sys_sage.Topology
Node = _py_sys_sage.Node
Memory = _py_sys_sage.Memory
Storage = _py_sys_sage.Storage
Chip = _py_sys_sage.Chip
Cache = _py_sys_sage.Cache
Subdivision = _py_sys_sage.Subdivision
Numa = _py_sys_sage.Numa
Core = _py_sys_sage.Core
Thread = _py_sys_sage.Thread
Qubit = _py_sys_sage.Qubit
QuantumBackend = _py_sys_sage.QuantumBackend
AtomSite = _py_sys_sage.AtomSite
Relation = _py_sys_sage.Relation
DataPath = _py_sys_sage.DataPath
CouplingMap = _py_sys_sage.CouplingMap
QuantumGate = _py_sys_sage.QuantumGate

if _py_sys_sage.HAS_PAPI:
    Metric = _py_sys_sage.Metric
    CpuMetrics = _py_sys_sage.CpuMetrics

# functions

ParseMt4g = _py_sys_sage.ParseMt4g
ParseMt4g_v1_x = _py_sys_sage.ParseMt4g_v1_x
ParseMt4g_v0_1 = _py_sys_sage.ParseMt4g_v0_1

parseHwlocOutput = _py_sys_sage.parseHwlocOutput
parseCccbenchOutput = _py_sys_sage.parseCccbenchOutput
parseCapsNumaBenchmark = _py_sys_sage.parseCapsNumaBenchmark
parseIQM = _py_sys_sage.parseIQM
exportToXml = _py_sys_sage.exportToXml
importFromXml = _py_sys_sage.importFromXml

if _py_sys_sage.HAS_PAPI:
    SS_PAPI_start = _py_sys_sage.SS_PAPI_start
    SS_PAPI_reset = _py_sys_sage.SS_PAPI_reset
    SS_PAPI_read = _py_sys_sage.SS_PAPI_read
    SS_PAPI_accum = _py_sys_sage.SS_PAPI_accum
    SS_PAPI_stop = _py_sys_sage.SS_PAPI_stop

del _py_sys_sage # remove internal library from namespace

# Architectural Concept and Design

## Workflow

The typical workflow is described in the figure below.

<center>
    ![workflow under sys-sage](images/workflow.drawio.png){html: width=60%, markdown: width=80%}
</center>

We use the term "user" to refer to any resource manager, daemon, user-side application, or any other entities that store or retrieve data from the library.

1. As part of the pre-run system discovery, the user measures and collects relevant topological information from available data sources
2. At application startup, the static configuration of the target architecture as well as the supplementary data from the previous step are imported into the library to materialize an initial system state.
3. During application runtime, topological information can be queried/updated/extended through the library's unified interface
4. Before the application terminates, the user may capture a snapshot of the system's state

## Conceptual Design

An overview of _sys-sage_'s design is given in the figure below.

<center>
    ![conceptual design of sys-sage](images/integration-architecture.drawio.png){html: width=35%, markdown: width=50%}
</center>

### Internal Representation

The hardware topology is modeled by an internal representation that aims at representing both the static, hierarchical and physical decomposition of hardware components as well as their dynamic, often non-hierarchical and logical interconnections.
It builds the foundation for the library by combining all data together into one central interface.

The physical hardware components are represented by the Component data structure, whose hierarchical layout form a **Component Tree** that naturaly describes the topologies basis.
Every piece of static or dynamic information attaches to or references Components within this tree, therefore being the primary access point to query topological information.
Although the design is inspired by [hwloc](https://www.open-mpi.org/projects/hwloc/)’s CPU-centric approach, _sys-sage_ significantly generalizes the concept to support a much broader and heterogeneous spectrum of hardware and system resources, thus extending far beyond CPU-centric modelling.
Users can interconnect Components arbitrarily, thus providing a high degree of freedom in expressing custom system configurations and layouts.
Furthermore, _sys-sage_ defines several ComponentTypes derived from distinct parts of computing hardware, with each being tailored towards holding the relevant characteristics and behaviors of its domain.
These types form a class inheritance hierarchy.
More information on the specific classes and their members can be found [here](@ref sys_sage::Component).

In contrast, a Relation is a complementary data structure that reflect different relationships and interactions between Components.
The full collection of Relations forms a **Relations Graph**, which captures information orthogonal to the Component Tree.
Relations are typically referenced through the participating Components, and accommodate a broad spectrum of information types including data-transfer characteristics, performance indicators, power consumption, application-specific metrics, or quantum-specific properties.
Moreover, a Component can participate in an arbitrary number of Relations, and multiple Relations can connect the same subset of Components to represent different dependencies or properties.
This open design provides high flexibility for diverse user-defined use-case scenarios.
Similar to Components, each Relation belongs to a specific RelationType, which allows for more specialized functionality targeted at a specific usage scenario.
Additionally, the RelationCategory provides semantic tagging of the specific Relation.
More information can be found [here](@ref sys_sage::Relation).

To illustrate the distinction, Components may represent individual caches along with their size, associativity, or observed hit rates from hardware counters.
Components will also be used to store a CPU core’s register file size and current frequency.
Relations extend this view by capturing relational properties between an arbitrary number of (often two: source-target) Components — for instance, cache-to-core load latencies across multiple cache levels or the fraction of cross-NUMA memory accesses between different NUMA domains.
The figures below showcases this distinction.

<center>
    ![sys-sage's Component Tree data structure](images/component-tree-dis.drawio.png){html: width=30%, markdown: width=40%}
    <br>
    ![sys-sage's Relations Graph data structure](images/relations-dis.drawio.png){html: width=30%, markdown: width=40%}
</center>

The one at the top shows the Component Tree consisting of different ComponentTypes marked by different colors, whereas the figure at the bottom demonstrates relations carrying different information as indicated by the different colors.

Refer to this [tutorial](tutorials/Tutorial_01.md) to get hands-on experience with _sys-sage_'s Component Tree and Relations Graph.

Moreover, in this [tutorial](tutorials/Tutorial_02.md) you can get a first look into the library's API and familiarize yourself with navigating through the Component Tree and retrieving topological information.

### Data Sources and Data Parsers

As mentioned before, _sys-sage_ collects topological information from external Data Sources to construct a full system representation.
The Data Sources referred to here constitute the data integration path stemming from pre-startup system discovery.
The raw data is stored within a file that can be the output of applications, benchmarks, scripts, user-defined configuration files, and other inputs.
These files are then uploaded to _sys-sage_ during startup, usually providing the base hardware topology and other information that does not change.
Furthermore, Data Sources can broadly be divided into two categories: Default Data Sources and Custom Data Sources.
Regarding the former, _sys-sage_ offers out-of-the-box integration for commonly used information.
The latter includes user-provided measurements, observations, or findings, which lie outside of the core of _sys-sage_ but can be integrated during the data upload process.

To upload a Data Source to _sys-sage_, a dedicated Data Parser is needed.
They interpret and process the given information and convert it to structures compatible with the library’s Internal Representation.
Similar to Data Sources, Data Parsers are divided into Default and Custom Data Parsers.
On one hand, Default Parsers are part of the library and can directly be used to upload the corresponding Default Data Sources.
On the other hand, are extensions implemented by users to parse their Custom Data Sources.
These are typically implemented by a single function that reads the data and merges it into the Internal Representation.

A list of Default Data Sources for which a Default Data Parser exists is given below.

<center>
    | Default Data Source | description |
    | ------------------- | -------- |
    | **hwloc** | CPU static topology |
    | **MT4G** | GPU static and dynamic topology information |
    | **IQM** | static topological information on IQM QPUs |
    | **cccbench** | CPU core-to-core communication latency benchmark |
    | **caps-numa-benchmark** | load latencies of CPU to NUMA and cross-NUMA memory accesses |
    | **MUSA** | static CPU topology in MUSA simulator input format |
    | **FIRESTARTER2**, **BabelSTREAM**, **NAS Parallel Benchmarks**, **High Performance Linpack**, **LULESH** | various performance benchmarks |
</center>

More details about the specific information these data sources provide can be found in the API documentation of their respective parsers.

Refer to this [tutorial](tutorials/Tutorial_03.md) to learn how to upload data sources and how to write your own parsers.

### User-specific Attributes

The information that _sys-sage_ carries can further be expanded to contain data that is specific to the current use-case scenario.
All Components and Relations can store additional attributes expressed as key-value-pairs provided by the user.
The flexibility to easily store arbitrary data in the Component Tree and the Relations Graph enable straight-forward customization and extension of the topology's model.

This [tutorial](tutorials/Tutorial_04.md) gives you a brief introduction into the usage of attributes.

### 3rd party extensions

Complementary to the data collected during the pre-run system discovery and uploaded to _sys-sage_ at application startup via the Data Parsers, live information about the application and the system state can be polled during runtime from a 3rd party library and integrated into _sys-sage_'s Internal Representation.
In contrast to Data Sources, these external libraries typically provide information about some dynamic system that can be accessed through a dedicated API, not a static file.
This means that integrating these libraries into _sys-sage_ usually requires wrapping the API around some logic -- instead of parsing a file -- to map the queried information to the Internal Representation.

Like with Default Data Sources, _sys-sage_ offers some pre-built 3rd party extensions:

<center>
    | 3rd Party API | description |
    | ------------- | ----------- |
    | **Intel PQoS** | resource sharing and cache isolation on Intel CPUs |
    | **NVIDIA MIG** | resource isolation and GPU partitioning on NVIDIA GPUs |
    | **proc_cpuinfo** | retrieve live information on CPU frequency from /proc/cpuinfo |
    | **PAPI** | capture hardware performance counters |
    | **QDMI** | retrieve QPU topological information and live system status |
</center>

On the one hand, 3rd party extensions enable a more richer system introspection involving insight into variable and dynamically changing system settings, while on the other hand _sys-sage_ envelops the standalone information in the overall context of the hardware topology.
To illustrate this symbiotic relationship, let us sketch the following use-case scenario:

Assume multiple applications run on a single node.
The compute cores are partitioned such that each application is assigned a fixed number of cores.
Moreover, the node is set under a power cap and therefore only has access to a limited power budget that it needs to distribute among the applications.
Let's say the "application manager" of the node favors to increase the clock frequency of cores assigned to compute-intensive applications while decreasing the clock frequency of memory-bound applications.
Hence, the application manager needs to know for every application how the change in frequency affects the performance.
For that reason, every application should monitor the respective performance counters (e.g. IPC, cache misses, stalled cycles, etc.) and the frequency of the cores.
Together these dynamic system properties can be used to express the change in performance, e.g.

\f[
\frac{\Delta \text{Performance}}{\Delta \text{Frequency}}
\f]

and reported to the application manager for the decision making.
Now this is how the _sys-sage_ integration helps:
_sys-sage_ can be used on both the application layer and the system management layer to have a unified system representation, making communication between these layers much simpler.
Moreover, by using the PAPI extension of _sys-sage_, the hardware performance counters can directly be correlated to the relevant cores on which the counters have been monitored on.
Furthermore, by using the proc_cpuinfo extension, we can capture the core's frequency and again associate them directly to the respective cores in the topology model.
This means that _sys-sage_ combines both the dynamic hardware counters and the clock frequency under the same abstraction of a CPU core, thus avoiding the need of having to coordinate both metrics in isolation.

Naturally, user-defined extensions can be added to _sys-sage_.
Have a look at this [tutorial](tutorials/Tutorial_05.md) to get an example.

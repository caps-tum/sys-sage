# sys-sage Data Sources

## hwloc

Hwloc is a library that provides a portable abstraction of the hierarchical topology of modern architectures It can be used to discover the detailed layout of the hardware. 
To use hwloc, simply include the header file in your source code and link against the library. The API provides functions to initialize the topology, load the topology from the system, and export it to an XML file. You can also query the topology information, such as the number of processing units, caches, and NUMA nodes,etc. 

to enable the **hwloc** data source, add '-DDS_HWLOC=on' to the cmake script

Example cmake command to build sys-sage with hwloc data source:
```
cmake -DCMAKE_INSTALL_PREFIX=../inst-dir -DDS_HWLOC=ON 


## mt4g

Retrieves compute and memory topology of GPUs (NVidia only for now). See the project's [git repository](https://github.com/caps-tum/mt4g), especially the Readme, for more information on how to use it.

To enable the **mt4g** data source, add `-DDS_MT4G=ON` to the cmake script.
You may also need to specify the nvcc compiler path `-DCMAKE_CUDA_COMPILER=/path/to/nvcc`.
Next, it is necessary to specify the target microarchitecture `-DNVIDIA_UARCH=<arch>`.
NVIDIA_UARCH options are: KEPLER, MAXWELL, PASCAL, VOLTA, TURING, AMPERE

Example cmake command to build sys-sage with mt4g data source:
```
cmake -DCMAKE_INSTALL_PREFIX=../inst-dir -DDS_MT4G=ON -DNVIDIA_UARCH="volta" -DCMAKE_CUDA_COMPILER=/usr/local/cuda-11.7/bin/nvcc ..
```

## caps-numa-benchmark

The CAPS NUMA Benchmark is a tool used for testing and proof of concept purposes to evaluate memory latency and bandwidth between different NUMA regions and CPUs. It measures memory latency and bandwidth between NUMA nodes and CPUs for a given memory size and array size. However, it is not recommended to use it as a benchmarking tool for comparative analysis due to its limitations and assumptions. Instead, it is intended to be used as a tool for ensuring that the system is working correctly and the NUMA architecture is being properly utilized.

To enable the **mt4g** data source, add `-DDS_NUMA=ON ` to the cmake script.
it's worth noting that some linux specific libraries are required. 

Example cmake command to build sys-sage with caps_numa_benchmark data source:
```
cmake -DCMAKE_INSTALL_PREFIX=../inst-dir -DDS_NUMA=ON ..




# Tutorial 03: Using Data Sources and Data Parsers

In this tutorial, we will upload Data Sources to _sys-sage_ in order to feed the library with topological information.
We try to automate the system topology discovery by letting _sys-sage_ build the initial Component Tree and Relations Graph, as we have done manually in the [1st tutorial](Tutorial_01.md).
Apart from that, we will write our own Data Parser to extend _sys-sage_'s topology model with some user-specific data.

## Automatatic Hardware Discovery

Instead of manually creating the Components and individually connecting them into the Component Tree, we can use existing tools to collect information about the architecture and upload them to _sys-sage_.
This information will then internally be processed to build an initial state for the Component Tree and the Relations Graph.
For this purpose, the primary Default Data Sources relevant here are [hwloc](https://github.com/open-mpi/hwloc) and [mt4g](https://github.com/caps-tum/mt4g).
In the following we will focus on these Data Sources.
Information about further sources can be found in the API documentation.

### hwloc -- CPU topology

The topological information regarding the CPU can be extracted from hwloc by using the `lstopo` CLI tool.
For instance,

```bash
lstopo topo.xml
```

will output an XML file named `topo.xml` in the current working directory, containing the relevant information.
By providing the path to this file to the hwloc Data Parser, we can upload the data to _sys-sage_ without much effort:

```cpp
sys_sage::Node *node = new sys_sage::Node(0);

// `xmlPath` contains the path to the XML file
if (sys_sage::parseHwlocOutput(node, xmlPath) == 0)
    return 1;
```

That's it.
The sys_sage::parseHwlocOutput function handles all the complications under the hood by parsing the file to extract the information and using it to construct the Component Tree representing the entire physical node layout.

To get a model of a distributed architecture with multiple nodes, you can run `lstopo` on each node and gather all XML files into one place, which can then be parsed one by one.
An example is shown below:

```cpp
sys_sage::Topology *topo = new sys_sage::Topology;

// assume `xmlPaths` is a vector of strings containing the paths to all gathered XML files
for (int i = 0; i < xmlPaths.size(); i++) {
    sys_sage::Node *node = new sys_sage::Node(i);
    if (sys_sage::parseHwlocOutput(node, xmlPaths[i]) != 0)
        topo->InsertChild(node);
}
```

### MT4G -- GPU topology

In a similar fashion, the `mt4g` CLI tool can be used to discover GPU topologies.
Running

```bash
mt4g -f topo.json
```

will provide data for the GPU (per default with device ID 0).
The data can be uploaded like this:

```cpp
if (sys_sage::ParseMt4g(node, jsonPath, gpuId) != 0)
    return 1;
```

Now, our Component Tree includes both topological information about the platform's CPUs and GPU.

## Writing a Custom Data Parser

Let's say we have run a cache line ping-pong microbenchmark to measure the cache line transfer rate between cores.
This information can be useful to evaluate cache coherence protocols, in which a core A exclusively modified a cache line and core B now requests access to the new data of the shared cache line from core A.
Moreover, since this information captures data movement, the sys_sage::DataPath relation would be a perfect fit to model this property.
Let us further assume the benchmark results are stored in a CSV file, where the format is as follows:

```
<coreA-ID>,<coreB-ID>,<latency>
```

We will provide the full implementation of the parser below and subsequently discuss it:

```cpp
std::tuple<int, int, double> extractInfo(const std::string &line)
{
    std::string::size_type start = 0;
    std::string::size_type pos = line.find(',', start);
    if (pos == std::string::npos)
        return {-1, -1, -1};

    int coreA_id = std::stoi(line.substr(start, pos - start));

    start = pos + 1;
    pos = line.find(',', start);
    if (pos == std::string::npos)
        return {-1, -1, -1};

    int coreB_id = std::stoi(line.substr(start, pos - start));

    start = pos + 1;
    pos = line.find(',', start);
    if (pos != std::string::npos)
        return {-1, -1, -1};

    double latency = std::stod(line.substr(start));

    return { coreA_id, coreB_id, latency };
}

int parseCacheLinePingPongBenchmark(sys_sage::Component *root, const std::string &csvPath)
{
    std::ifstream file (csvPath);
    if (!file)
        return 1;

    std::string line;
    while (std::getline(file, line)) {
        auto [coreA_id, coreB_id, latency] = extractInfo(line);
        if (coreA_id == -1 || coreB_id == -1 || latency == -1)
            return 1;

        sys_sage::Component *coreA = root->GetDescendantById(coreA_id, sys_sage::ComponentType::Core);
        sys_sage::Component *coreB = root->GetDescendantById(coreB_id, sys_sage::ComponentType::Core);

        if (coreA == nullptr || coreB == nullptr)
            return 1;

        sys_sage::DataPath *dp = new sys_sage::DataPath(coreA, coreB, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathCategory::Datatransfer);
        dp->SetLatency(latency);
    }

    return 0;
}
```

The function `parseCacheLinePingPongBenchmark` takes a pointer to a Component that represents the root of the (sub-)tree and a string containing the path to the output Data Source file.
While reading every line of the file one by one, it extracts the comma seperated values by calling the helper function `extractInfo`.
Moreover, we use the method sys_sage::Component::GetDescendantById to search for a core with the given ID in the subtree spanned by `root`.
If no such cores could be found, we return with an error, assuming that the file is corrupted.
Afterwards, we create a new DataPath to model this relational property by setting its latency to the latency retrieved from the benchmark.

This way, we have successfully integrated the benchmark results into our topology model.

## Putting Everything Together

The complete code for this tutorial is given below:

```cpp
#include <sys-sage.hpp>
#include <iostream>

std::tuple<int, int, double> extractInfo(const std::string &line)
{
    std::string::size_type start = 0;
    std::string::size_type pos = line.find(',', start);
    if (pos == std::string::npos)
        return {-1, -1, -1};

    int coreA_id = std::stoi(line.substr(start, pos - start));

    start = pos + 1;
    pos = line.find(',', start);
    if (pos == std::string::npos)
        return {-1, -1, -1};

    int coreB_id = std::stoi(line.substr(start, pos - start));

    start = pos + 1;
    pos = line.find(',', start);
    if (pos != std::string::npos)
        return {-1, -1, -1};

    double latency = std::stod(line.substr(start));

    return { coreA_id, coreB_id, latency };
}

int parseCacheLinePingPongBenchmark(sys_sage::Component *root, const std::string &csvPath)
{
    std::ifstream file (csvPath);
    if (!file)
        return 1;

    std::string line;
    while (std::getline(file, line)) {
        auto [coreA_id, coreB_id, latency] = extractInfo(line);
        if (coreA_id == -1 || coreB_id == -1 || latency == -1)
            return 1;

        sys_sage::Component *coreA = root->GetDescendantById(coreA_id, sys_sage::ComponentType::Core);
        sys_sage::Component *coreB = root->GetDescendantById(coreB_id, sys_sage::ComponentType::Core);

        if (coreA == nullptr || coreB == nullptr)
            return 1;

        sys_sage::DataPath *dp = new sys_sage::DataPath(coreA, coreB, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathCategory::Datatransfer);
        dp->SetLatency(latency);
    }

    return 0;
}

int main(int argc, const char **argv)
{
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << "<path_to_hwloc_xml_file> <path_to_mt4g_json_file> <path_to_benchmark_csv_file>\n";
        return 1;
    }

    std::string xmlPath = argv[1];
    std::string jsonPath = argv[2];
    std::string csvPath = argv[3];

    sys_sage::Node *node = new sys_sage::Node(0);

    if (sys_sage::parseHwlocOutput(node, xmlPath) != 0)
        return 1;

    int gpuId = 0;
    if (sys_sage::ParseMt4g(node, jsonPath, gpuId) != 0)
        return 1;

    if (parseCacheLinePingPongBenchmark(node, csvPath) != 0)
        return 1;

    node->PrintSubtree();
    node->PrintRelationsInSubtree();

    sys_sage::Component::DeleteSubtree(node);

    return 0;
}
```

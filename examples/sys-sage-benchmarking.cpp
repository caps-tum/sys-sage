

#include <iostream>
//#include <hwloc.h>
#include <chrono>

#include "sys-sage.hpp"

////////////////////////////////////////////////////////////////////////
//PARAMS TO SET
#define TIMER_WARMUP 32
#define TIMER_REPEATS 128

////////////////////////////////////////////////////////////////////////
using namespace std::chrono;


//int hwloc_dump_xml(const char *filename);
uint64_t get_timer_overhead(int repeats, int warmup);


//this file benchmarks and prints out performance information about basic operations with sys-sage
int main(int argc, char *argv[])
{
    std::string path_prefix(argv[0]);
    std::size_t found = path_prefix.find_last_of("/\\");
    path_prefix=path_prefix.substr(0,found) + "/";
    string topoPath = path_prefix + "example_data/skylake_hwloc.xml";
    string bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv";
    string mt4gPath = path_prefix + "example_data/pascal_gpu_topo.csv";

    high_resolution_clock::time_point t_start, t_end;
    uint64_t timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);

    Topology* t = new Topology();
    vector<Component*> hwlocComponentList, mt4gComponentList, allComponentList;

    //time create new component
    t_start = high_resolution_clock::now();
    Node* n = new Node(t,1);
    t_end = high_resolution_clock::now();
    uint64_t time_createNewComponent = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;

    //time hwloc_parser
    t_start = high_resolution_clock::now();
    int ret = parseHwlocOutput(n, topoPath);
    t_end = high_resolution_clock::now();
    uint64_t time_parseHwlocOutput = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;
    if(ret != 0) {//adds topo to a next node
        cout << "failed parsing hwloc" << endl;
        return 1;
    }

    //time get a vector with all Components (of hwloc parser)
    hwlocComponentList.clear();
    t_start = high_resolution_clock::now();
    n->GetSubtreeNodeList(&hwlocComponentList);
    t_end = high_resolution_clock::now();
    uint64_t time_GetHwlocSubtreeNodeList = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;

    //time caps-numa-benchmark parser
    t_start = high_resolution_clock::now();
    ret = parseCapsNumaBenchmark((Component*)n, bwPath, ";");
    t_end = high_resolution_clock::now();
    uint64_t time_parseCapsNumaBenchmark = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;
    if(ret != 0){
        cout << "failed parsing caps-numa-benchmark" << endl;
        return 1;
    }

    //get num caps-benchmark DataPaths
    int caps_dataPaths = 0;
    vector<DataPath*>* capsDataPaths;
    for(Component* gpu_c: hwlocComponentList)
    {
        capsDataPaths = gpu_c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        caps_dataPaths += capsDataPaths->size();
    }

    //get size of hwloc representation + caps-benchmark DataPaths
    unsigned hwloc_component_size=0;
    unsigned caps_numa_dataPathSize=0;
    unsigned total_size = n->GetTopologySize(&hwloc_component_size, &caps_numa_dataPathSize);

    //for NUMA 0 get NUMA with min BW
    Numa * numa = (Numa*)n->FindSubcomponentById(0, SYS_SAGE_COMPONENT_NUMA);
    if(numa==NULL){ cerr << "numa 0 not found in sys-sage" << endl; return 1;}
    unsigned int max_bw = 0;
    Component* max_bw_component = NULL;
    t_start = high_resolution_clock::now();
    vector<DataPath*>* dp = numa->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    for(auto it = std::begin(*dp); it != std::end(*dp); ++it) {
        if( (*it)->GetBw() > max_bw ){
            max_bw = (*it)->GetBw();
            max_bw_component = (*it)->GetTarget();
        }
    }
    t_end = high_resolution_clock::now();
    uint64_t time_getNumaMaxBw = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;

    //time mt4g parser
    Chip* gpu = new Chip(n, 100, "GPU");
    t_start = high_resolution_clock::now();
    ret = parseGpuTopo(gpu, mt4gPath, ";");
    t_end = high_resolution_clock::now();
    uint64_t time_parseMt4g = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;
    if(ret != 0){
        cout << "failed parsing mt4g" << endl;
        return 1;
    }

    //time get a vector with all Components (of mt4g parser)
    mt4gComponentList.clear();
    t_start = high_resolution_clock::now();
    gpu->GetSubtreeNodeList(&mt4gComponentList);
    t_end = high_resolution_clock::now();
    uint64_t time_GetMt4gSubtreeNodeList = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;

    //time get a vector with all Components
    allComponentList.clear();
    t_start = high_resolution_clock::now();
    t->GetSubtreeNodeList(&allComponentList);
    t_end = high_resolution_clock::now();
    uint64_t time_GetAllComponentsList = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;

    //get num mt4g DataPaths
    int mt4g_dataPaths = 0;
    vector<DataPath*>* componentDataPaths;
    for(Component* gpu_c: mt4gComponentList)
    {
        componentDataPaths = gpu_c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        mt4g_dataPaths += componentDataPaths->size();
    }

    /////////////////print results
    cout << "time_parseHwlocOutput, " << time_parseHwlocOutput;
    cout << ", time_parseCapsNumaBenchmark, " << time_parseCapsNumaBenchmark;
    cout << ", time_parseMt4g, " << time_parseMt4g;
    cout << ", time_GetHwlocSubtreeNodeList, " << time_GetHwlocSubtreeNodeList;
    cout << ", time_GetMt4gSubtreeNodeList, " << time_GetMt4gSubtreeNodeList;
    cout << ", time_GetAllComponentsList, " << time_GetAllComponentsList;

    cout << ", hwloc_components, " << hwlocComponentList.size();
    cout << ", caps_dataPaths, " << caps_dataPaths;
    cout << ", mt4g_components, " << mt4gComponentList.size();
    cout << ", mt4g_dataPaths, "<< mt4g_dataPaths;
    cout << ", all_components, " << allComponentList.size() ;

    cout << ", time_getNumaMaxBw, " << time_getNumaMaxBw; //<< "; bw; " << max_bw << "; ComponentId; " << max_bw_component->GetId() << endl;
    cout << ", time_createNewComponent, " << time_createNewComponent;

    cout << ", hwloc_component_size[B], " << hwloc_component_size;
    cout << ", caps_numa_dataPathSize[B], " << caps_numa_dataPathSize;
    cout << ", total_size, " << total_size;

    cout << endl;

    return 0;
}

uint64_t get_timer_overhead(int repeats, int warmup)
{
    high_resolution_clock::time_point t_start, t_end;
    uint64_t time = 0;
    //uint64_t time_arr[10];
    for(int i=0; i<repeats+warmup; i++)
    {
        t_start = high_resolution_clock::now();
        t_end = high_resolution_clock::now();
        if(i>=warmup)
            time += t_end.time_since_epoch().count()-t_start.time_since_epoch().count();
    }
    time = time/repeats;
    return time;
}

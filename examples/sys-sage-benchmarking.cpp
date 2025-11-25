

#include <cstdint>
#include <iostream>
// #include <hwloc.h>
#include <chrono>
#include <libxml2/libxml/parser.h>
#include <sys/types.h>

#include "sys-sage.hpp"

using namespace sys_sage;

////////////////////////////////////////////////////////////////////////
// PARAMS TO SET
#define TIMER_WARMUP 32
#define TIMER_REPEATS 128

////////////////////////////////////////////////////////////////////////
using namespace std::chrono;

using std::cout;
using std::endl;

//int hwloc_dump_xml(const char *filename);
uint64_t get_timer_overhead(int repeats, int warmup);

int search_simple(std::string k, void *value, std::string *ret_value_str) {
  if (!k.compare("benchmark")) {
    *ret_value_str = *(std::string *)value;
    return 1;
  }
  return 0;
};
int search_complex(std::string k, void *value, xmlNodePtr n) {
  if (!k.compare("complex")) {
    int *val = (int *)value;

    xmlNodePtr attr_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
    xmlNewProp(attr_node, (const unsigned char *)"key",
               (const unsigned char *)k.c_str());
    xmlNewProp(attr_node, (const unsigned char *)"value",
               (const unsigned char *)std::to_string(*val).c_str());
    xmlAddChild(n, attr_node);
    return 1;
  }
  return 0;
};


void *imp_search_simple(xmlNodePtr n) {
  if (xmlHasProp(n, (const xmlChar *)"name") &&
      xmlHasProp(n, (const xmlChar *)"value")) {
    const unsigned char *v = xmlGetProp(n, (const unsigned char *)"name");
    std::string key(reinterpret_cast<char const *>(v));
    v = xmlGetProp(n, (const unsigned char *)"value");
    std::string value(reinterpret_cast<char const *>(v));
    return new std::string(value);
  } else {
    return NULL;
  }
}

int imp_search_complex(xmlNodePtr n, Component *c) {
  if (xmlHasProp(n, (const xmlChar *)"key") &&
      xmlHasProp(n, (const xmlChar *)"value")) {
    const unsigned char *v = xmlGetProp(n, (const unsigned char *)"key");
    std::string key(reinterpret_cast<char const *>(v));
    v = xmlGetProp(n, (const unsigned char *)"value");
    int* value = new int(std::stoi(reinterpret_cast<char const *>(v)));
    c->attrib[key] = (void*) value;
    return 1;
  } else {
    return 0;
  }
}


//this file benchmarks and prints out performance information about basic operations with sys-sage
int main(int argc, char *argv[])
{
    std::string path_prefix(argv[0]);
    std::size_t found = path_prefix.find_last_of("/\\");
    path_prefix=path_prefix.substr(0,found) + "/";
    std::string xmlPath = path_prefix + "example_data/skylake_hwloc.xml";
    std::string bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv";
    std::string mt4gPath = path_prefix + "example_data/pascal_gpu_topo.csv";

    high_resolution_clock::time_point t_start, t_end;
    uint64_t timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);

    Topology* t = new Topology();
    Node *n = new Node(t, 1);
    std::vector<Component*> hwlocComponentList, mt4gComponentList, allComponentList;

    // time exportToXML
    uint64_t time_exportToXml = UINT64_MAX;

    for (int i = 0; i < 1000; i++) {
        t_start = high_resolution_clock::now();
        exportToXml(t, "test.xml", search_simple, search_complex);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_exportToXml) {
            time_exportToXml = time;
        }   
    }

    uint64_t time_createNewComponent = UINT64_MAX;

    for (int i = 0; i < 1000; i++) {
        n->Delete(false);
        t_start = high_resolution_clock::now();
        n = new Node(t, 1);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_createNewComponent) {
        time_createNewComponent = time;
        }
    }
    //time importFromXml
    uint64_t time_importFromXml = UINT64_MAX;
    for (int i = 0; i < 1000; i++) {
        t_start = high_resolution_clock::now();
        Component *f = importFromXml("test.xml",NULL,NULL);
        t_end = high_resolution_clock::now();
        f->Delete(true);
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_importFromXml) {
        time_importFromXml = time;
        }
    }
    //time update an attribute
    
    uint64_t total_time_updateAttribute = 0;

    for (int i = 0; i < 1000000; i++) {
        t_start = high_resolution_clock::now();
        [[ maybe_unused ]] int* l = new int(1);
        if (n->attrib.find("test") != n->attrib.end()) {
        int* oldValue = static_cast<int*>(n->attrib["test"]);
        delete oldValue;  // Free the old value
        }
        int* newValue = new int(100);  // Allocate new value
        n->attrib["test"] = static_cast<void*>(newValue);  // Update the map
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        total_time_updateAttribute += time;
    }

    uint64_t avg_time_updateAttribute = total_time_updateAttribute / 1000000;

    //time read attribute

    uint64_t total_time_readAttribute = 0;

    for (int i = 0; i < 1000000; i++) {
        t_start = high_resolution_clock::now();
        [[ maybe_unused ]] int* l = static_cast<int*>(n->attrib["test"]);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        total_time_readAttribute += time;
    }

    uint64_t avg_time_readAttribute = total_time_readAttribute / 1000000;

    // time hwloc_parser

    uint64_t time_parseHwlocOutput = UINT64_MAX;
    int ret = parseHwlocOutput(n, xmlPath);
    t_start = high_resolution_clock::now();
    ret = parseHwlocOutput(n, xmlPath);
    t_end = high_resolution_clock::now();
    uint64_t time = t_end.time_since_epoch().count() -
                    t_start.time_since_epoch().count() - timer_overhead;
    time_parseHwlocOutput = time;
    if (ret != 0) {
        cout << "failed parsing hwloc" << endl;
        return 1;
    }

    // time get a vector with all Components (of hwloc parser)
    uint64_t time_GetHwlocSubtreeNodeList = UINT64_MAX;
    for (int i = 0; i < 1000; i++) {
        hwlocComponentList.clear();
        t_start = high_resolution_clock::now();
        n->GetComponentsInSubtree(&hwlocComponentList);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_GetHwlocSubtreeNodeList) {
        time_GetHwlocSubtreeNodeList = time;
        }
    }

    // time caps-numa-benchmark parser
    uint64_t time_parseCapsNumaBenchmark = UINT64_MAX;
    t_start = high_resolution_clock::now();
    ret = parseCapsNumaBenchmark((Component *)n, bwPath, ";");
    t_end = high_resolution_clock::now();
    time_parseCapsNumaBenchmark = t_end.time_since_epoch().count() -
                                    t_start.time_since_epoch().count() -
                                    timer_overhead;
    if (ret != 0) {
        cout << "failed parsing caps-numa-benchmark" << endl;
        return 1;
    }
    
    //get num caps-benchmark DataPaths
    int caps_dataPaths = 0;
    std::vector<DataPath*> capsDataPaths;
    for(Component* gpu_c: hwlocComponentList)
    {
        capsDataPaths = gpu_c->GetAllDataPaths(sys_sage::DataPathType::Any, sys_sage::DataPathDirection::Outgoing);
        // capsDataPaths = gpu_c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        caps_dataPaths += capsDataPaths.size();
    }


    // get size of hwloc representation + caps-benchmark DataPaths
    unsigned hwloc_component_size = 0;
    unsigned caps_numa_dataPathSize = 0;
    unsigned total_size = n->GetTopologySize(&hwloc_component_size, &caps_numa_dataPathSize);
    cout << "hwloc_component_size, " << hwloc_component_size
        << ", caps_numa_dataPathSize, " << caps_numa_dataPathSize
        << ", total_size, " << total_size << endl;


    //for NUMA 0 get NUMA with min BW
    uint64_t time_getNumaMaxBw = UINT64_MAX;
    Numa * numa = (Numa*)n->GetSubcomponentById(0, sys_sage::ComponentType::Numa);
    if(numa==NULL){ std::cerr << "numa 0 not found in sys-sage" << endl; return 1;}
    unsigned int max_bw = 0;
    Component* max_bw_component = NULL;
    t_start = high_resolution_clock::now();
    std::vector<DataPath*> dp_vec = numa->GetAllDataPaths(sys_sage::DataPathType::Any, sys_sage::DataPathDirection::Outgoing);
    // vector<DataPath*>* dp = numa->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    for(DataPath* dp : dp_vec)
    {
        if( dp->GetBandwidth() > max_bw ){
            max_bw = dp->GetBandwidth();
            max_bw_component = dp->GetTarget();
        }        
    }
    t_end = high_resolution_clock::now();
    time = t_end.time_since_epoch().count() -  t_start.time_since_epoch().count() - timer_overhead;
    if (time < time_getNumaMaxBw) {
        time_getNumaMaxBw = time;
    }


    // time mt4g parser
    Chip *gpu = new Chip(n, 100, "GPU");
    uint64_t time_parseMt4g = UINT64_MAX;

    t_start = high_resolution_clock::now();
    ret = ParseMt4g_v0_1(gpu, mt4gPath, ";");
    t_end = high_resolution_clock::now();
    time_parseMt4g = t_end.time_since_epoch().count() -
                    t_start.time_since_epoch().count() - timer_overhead;
    if (ret != 0) {
        cout << "failed parsing mt4g" << endl;
        return 1;
    }

    // time get a vector with all Components (of mt4g parser)
    uint64_t time_GetMt4gSubtreeNodeList = UINT64_MAX;

    for (int i = 0; i < 100; i++) {
        mt4gComponentList.clear();
        t_start = high_resolution_clock::now();
        gpu->GetComponentsInSubtree(&mt4gComponentList);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() -
                        t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_GetMt4gSubtreeNodeList) {
        time_GetMt4gSubtreeNodeList = time;
        }
    }

    // time get a vector with all Components
    uint64_t time_GetAllComponentsList = UINT64_MAX;

    for (int i = 0; i < 1000; i++) {
        allComponentList.clear();
        t_start = high_resolution_clock::now();
        t->GetComponentsInSubtree(&allComponentList);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_GetAllComponentsList) {
            time_GetAllComponentsList = time;
            [[ maybe_unused ]] uint64_t time_GetAllComponentsList = t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-timer_overhead;
        }
    }
    //get num mt4g DataPaths
    int mt4g_dataPaths = 0;
    std::vector<DataPath*> componentDataPaths;
    for(Component* gpu_c: mt4gComponentList)
    {
        componentDataPaths = gpu_c->GetAllDataPaths(sys_sage::DataPathType::Any, sys_sage::DataPathDirection::Outgoing);
        // componentDataPaths = gpu_c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        mt4g_dataPaths += componentDataPaths.size();
    }


    /////////////////print results
    cout << "time_exportToXml, "
        << duration_cast<nanoseconds>(nanoseconds(time_exportToXml)).count()
        << " ns" << endl;
    cout << "time_importFromXml, "
        << duration_cast<nanoseconds>(nanoseconds(time_importFromXml)).count()
        << " ns" << endl;
    cout << "time_avg_updateAttribute, "
        << duration_cast<nanoseconds>(nanoseconds(avg_time_updateAttribute)).count()
        << " ns" << endl;
    cout << "time_avg_readAttribute, "
        << duration_cast<nanoseconds>(nanoseconds(avg_time_readAttribute)).count()
        << " ns" << endl;
    cout << "time_parseHwlocOutput, "
        << duration_cast<nanoseconds>(nanoseconds(time_parseHwlocOutput)).count()
        << " ns" << endl;
    cout << ", time_parseCapsNumaBenchmark, "
        << duration_cast<nanoseconds>(nanoseconds(time_parseCapsNumaBenchmark))
                .count()
        << " ns" << endl;
    cout << ", time_parseMt4g, "
        << duration_cast<nanoseconds>(nanoseconds(time_parseMt4g)).count()
        << " ns" << endl;
    cout << ", time_GetHwlocSubtreeNodeList, "
        << duration_cast<nanoseconds>(nanoseconds(time_GetHwlocSubtreeNodeList))
                .count()
        << " ns" << endl;
    cout << ", time_GetMt4gSubtreeNodeList, "
        << duration_cast<nanoseconds>(nanoseconds(time_GetMt4gSubtreeNodeList))
                .count()
        << " ns" << endl;
    cout << ", time_GetAllComponentsList, "
        << duration_cast<nanoseconds>(nanoseconds(time_GetAllComponentsList))
                .count()
        << " ns" << endl;

    cout << ", hwloc_components, " << hwlocComponentList.size() << endl;
    cout << ", caps_dataPaths, " << caps_dataPaths << endl;
    cout << ", mt4g_components, " << mt4gComponentList.size() << endl;
    cout << ", mt4g_dataPaths, " << mt4g_dataPaths << endl;
    cout << ", all_components, " << allComponentList.size() << endl;

    cout << ", time_getNumaMaxBw, "
        << duration_cast<nanoseconds>(nanoseconds(time_getNumaMaxBw)).count()
        << " ns" << endl;
    if (false)
        cout << "; bw; " << max_bw << "; ComponentId; " << max_bw_component->GetId()
            << endl;
    cout << ", time_createNewComponent, "
        << duration_cast<nanoseconds>(nanoseconds(time_createNewComponent))
                .count()
        << " ns" << endl;

    cout << ", hwloc_component_size[B], " << hwloc_component_size << endl;
    cout << ", caps_numa_dataPathSize[B], " << caps_numa_dataPathSize << endl;
    cout << ", total_size, " << total_size << endl;

    cout << endl;

    return 0;
}

uint64_t get_timer_overhead(int repeats, int warmup) {
    high_resolution_clock::time_point t_start, t_end;
    uint64_t time = 0;
    // uint64_t time_arr[10];
    for (int i = 0; i < repeats + warmup; i++) {
        t_start = high_resolution_clock::now();
        t_end = high_resolution_clock::now();
        if (i >= warmup)
        time +=
            t_end.time_since_epoch().count() - t_start.time_since_epoch().count();
    }
    time = time / repeats;
    return time;
}

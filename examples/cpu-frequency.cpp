#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <tuple>

#include "sys-sage.hpp"

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " [hwloc xml path] [xml output path/name]" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    string topoPath;
    string output_name = "sys-sage_cpu-frequency.xml";
    if(argc < 2){
        std::string path_prefix(argv[0]);
        std::size_t found = path_prefix.find_last_of("/\\");
        path_prefix=path_prefix.substr(0,found) + "/";
        topoPath = path_prefix + "example_data/skylake_hwloc.xml";
    }
    else if(argc == 2){
        topoPath = argv[1];
    }
    else if(argc == 3){
        topoPath = argv[1];
        output_name = argv[2];
    }
    else{
        usage(argv[0]);
        return 1;
    }

    //create root Topology and one node
    Topology* topo = new Topology();
    Node* n = new Node(topo, 1);

    cout << "-- Parsing Hwloc output from file " << topoPath << endl;
    if(parseHwlocOutput(n, topoPath) != 0) { //adds topo to a next node
        usage(argv[0]);
        return 1;
    }
    cout << "-- End parseHwlocOutput" << endl;

    cout << "-- Refresh frequency on core 1 (and do not store the timestamp). ";
    Core* c1 = (Core*)n->FindSubcomponentById(1, SYS_SAGE_COMPONENT_CORE);
    if(c1 != NULL)
    {
        c1->RefreshFreq(false);//
        cout << "Frequency: " << c1->GetFreq() << endl;
    }

    cout << "-- Refresh frequency on all cores of Node 1(and store the timestamp). " << endl;
    //Frequency gets stored in attrib freq_history (value of type std::vector<std::tuple<long long=timestamp,double=frequency in MHz>>)
    int repeat = 10;
    for(int i = 0; i<repeat; i++)
    {
        n->RefreshCpuCoreFrequency(true);
        usleep(100000);//100 ms
    }

    cout << "-- Print out frequency history on core 1 of Node 1. " << endl;
    std::vector<std::tuple<long long,double>>* fh = (std::vector<std::tuple<long long,double>>*)c1->attrib["freq_history"];
    for(auto [ ts,freq ] : *fh)
    {
        cout << "    ts: " << ts << " frequency[MHz]: " << freq << endl;
    }

    cout << "-- Export all information to xml " << output_name << endl;
    exportToXml(topo, output_name);

    return 0;
}

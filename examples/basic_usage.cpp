#include <iostream>
#include <filesystem>

#include "sys-sage.hpp"

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <hwloc xml path> <caps-numa-benchmark csv path>" << std::endl;
    std::cerr << "       or" << std::endl;
    std::cerr << "       " << argv0 << " (uses predefined paths which may be incorrect.)" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    string xmlPath;
    string bwPath;
    if(argc < 2){
        std::string path_prefix(argv[0]);
        std::size_t found = path_prefix.find_last_of("/\\");
        path_prefix=path_prefix.substr(0,found) + "/";
        xmlPath = path_prefix + "example_data/skylake_hwloc.xml";
        bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv";
    }
    else if(argc == 3){
        xmlPath = argv[1];
        bwPath = argv[2];
    }
    else{
        usage(argv[0]);
        return 1;
    }

    //create root Topology and one node
    Topology* topo = new Topology();
    Node* n = new Node(topo, 1);

    
    cout << "-- Parsing Hwloc output from file " << xmlPath << endl;
    if(parseHwlocOutput(n, xmlPath) != 0) { //adds topo to a next node
        usage(argv[0]);
        return 1;
    }
    cout << "-- End parseHwlocOutput" << endl;

    cout << "Total num HW threads: " << topo->GetNumThreads() << endl;

    cout << "---------------- Printing the whole tree ----------------" << endl;
    topo->PrintSubtree();
    cout << "----------------                     ----------------" << endl;
    cout << "-- Parsing CapsNumaBenchmark output from file " << bwPath << endl;
    if(parseCapsNumaBenchmark((Component*)n, bwPath, ";") != 0){
        cout << "failed parsing caps-numa-benchmark" << endl;
        usage(argv[0]);
        return 1;
    }
    cout << "-- End parseCapsNumaBenchmark" << endl;

    cout << "---------------- Printing all DataPaths ----------------" << endl;
    n->PrintAllDataPathsInSubtree();
    cout << "----------------                        ----------------" << endl;

    string output_name = "sys-sage_sample_output.xml";
    cout << "-------- Exporting as XML to " << output_name << " --------" << endl;
    exportToXml(topo, output_name);

    cout << "--------Clearing up the tree--------" << endl;
    topo->Delete(true);
    return 0;
}

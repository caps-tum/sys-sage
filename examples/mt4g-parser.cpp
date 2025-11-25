#include <iostream>
#include <filesystem>

#include "sys-sage.hpp"

using namespace sys_sage;

using std::cout;
using std::endl;

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <mt4g output path>" << std::endl;
    std::cerr << "       or" << std::endl;
    std::cerr << "       " << argv0 << " (uses predefined paths which may be incorrect.)" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    std::string gpuTopoPath;
    if(argc < 2){
        std::string path_prefix(argv[0]);
        std::size_t found = path_prefix.find_last_of("/\\");
        path_prefix=path_prefix.substr(0,found) + "/";
        gpuTopoPath = path_prefix + "example_data/ampere_gpu_topo.csv";
    }
    else if(argc == 2){
        gpuTopoPath = argv[1];
    }
    else{
        usage(argv[0]);
        return 1;
    }

    //create root Topology and one node
    Topology* topo = new Topology();
    Node* n = new Node(topo,1);

    cout << "-- Parsing mt4g output from file " << gpuTopoPath << endl;
    if(parseMt4gTopo((Component*)n, gpuTopoPath, 0, ";") != 0) { //adds topo to a next node
        return 1;
    }
    cout << "-- End parseGpuTopo" << endl;

    cout << "Total num GPU cores: " << topo->CountDescendantsByType(sys_sage::ComponentType::Thread) << endl;

    std::string output_name = "sys-sage_gpu_sample_output.xml";
    cout << "-------- Exporting as XML to " << output_name << " --------" << endl;
    
    //topo->DeleteSubtree();
    exportToXml(topo, output_name);
    
    topo->Delete(true);

    return 0;
}

#include <iostream>
#include <filesystem>

#include "sys-sage.hpp"

using namespace sys_sage;

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <gpu-topo path>" << std::endl;
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

    std::cout << "-- Parsing gpu-topo benchmark from file " << gpuTopoPath << std::endl;
    if(parseMt4gTopo((Component*)n, gpuTopoPath, 0, ";") != 0) { //adds topo to a next node
        return 1;
    }
    std::cout << "-- End parseGpuTopo" << std::endl;
    Chip * gpu = (Chip*)n->GetChild(0);
    gpu->UpdateMIGSettings();

    std::cout << "-- Current MIG settings reflected." << std::endl;
    std::string output_name = "sys-sage_sample_output.xml";
    std::cout << "-------- Exporting as XML to " << output_name << " --------" << std::endl;
    exportToXml(topo, output_name);

    std::vector<Component*> memories;
    gpu->FindDescendantsByType(&memories, sys_sage::ComponentType::Memory);
    for(Component* m: memories){
        std::cout << "Memory with id " << ((Memory*)m)->GetId() << ": MIG size " << ((Memory*)m)->GetMIGSize() << " (total size " << ((Memory*)m)->GetSize() << ")" << std::endl;
    }
    if(memories.size() == 0){
        std::cout << "NO MEMORIES FOUND ON THE GPU" << std::endl;
    }
    std::cout << "Number of MIG available SMs: " << gpu->GetMIGNumSMs() << std::endl;
    std::cout << "Number of MIG available GPU cores: " << gpu->GetMIGNumCores() << std::endl;

    return 0;
}
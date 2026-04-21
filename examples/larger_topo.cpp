#include <sys-sage.hpp>
#include <iostream>
#include <string>

using namespace sys_sage;

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    //create root Topology and one node
    Topology* topo = new Topology();
    int tot_nodes=8;
    Node* n = new Node(1);
    n->SetParent((Component*)topo);
    topo->InsertChild((Component*)n);

    cout << "create topology..." << endl;
    std::string path_prefix(argv[0]);
    std::size_t found = path_prefix.find_last_of("/\\");
    path_prefix=path_prefix.substr(0,found) + "/";
    std::string xmlPath = "example_data/skylake_hwloc.xml";
    std::string bwPath = "example_data/skylake_caps_numa_benchmark.csv";
    for(int n_idx=0; n_idx<tot_nodes; n_idx++)
    {
        Node* n = new Node(n_idx);
        n->SetParent((Component*)topo);
        topo->InsertChild((Component*)n);
        if(parseHwlocOutput(n, path_prefix+xmlPath) != 0)
        {
            cout << "error parsing hwloc in path " << path_prefix+xmlPath << endl;
            return 1;
        }
        if(parseCapsNumaBenchmark((Component*)n, path_prefix+bwPath, ";") != 0)
        {
            cout << "failed parsing caps-numa-benchmark in path " << path_prefix+bwPath << endl;
            return 1;
        }
    }

    std::string output_name = "sys-sage_sample_output.xml";
    cout << "Exporting as XML to " << output_name << endl;
    exportToXml(topo, output_name);

    return 0;
}

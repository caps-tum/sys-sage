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
    std::string xmlPath = EXAMPLE_DIR + std::string("/skylake_hwloc.xml");
    std::string bwPath = EXAMPLE_DIR + std::string("/skylake_caps_numa_benchmark.csv");

    for(int n_idx=0; n_idx<tot_nodes; n_idx++)
    {
        Node* n = new Node(n_idx);
        n->SetParent((Component*)topo);
        topo->InsertChild((Component*)n);
        if(parseHwlocOutput(n, xmlPath) != 0)
        {
            cout << "error parsing hwloc in path " << xmlPath << endl;
            return 1;
        }
        if(parseCapsNumaBenchmark((Component*)n, bwPath, ";") != 0)
        {
            cout << "failed parsing caps-numa-benchmark in path " << bwPath << endl;
            return 1;
        }
    }

    std::string output_name = "sys-sage_sample_output.xml";
    cout << "Exporting as XML to " << output_name << endl;
    exportToXml(topo, output_name);

    return 0;
}

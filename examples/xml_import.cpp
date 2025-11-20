
#include "sys-sage.hpp"
#include <map>
#include <string>

using std::cout;
using std::endl;

using namespace sys_sage;


void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <hwloc xml path> <caps-numa-benchmark csv path>" << std::endl;
    std::cerr << "       or" << std::endl;
    std::cerr << "       " << argv0 << " (uses predefined paths which may be incorrect.)" << std::endl;
    return;
}

int main(int argc, char *argv[]) 
{
    std::string xmlPath;
    std::string bwPath;
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

    // create root Topology and one node
    Topology *topo = new Topology();
    Node *n = new Node(topo, 1);

    cout << "-- Parsing Hwloc output from file " << xmlPath << endl;
    parseHwlocOutput(n, xmlPath);

    cout << "-- Parsing CapsNumaBenchmark output from file " << bwPath << endl;
    parseCapsNumaBenchmark((Component*)n, bwPath, ";");


    std::vector<Component *> c_orig = topo->FindDescendantsByType(ComponentType::Any);

    exportToXml(topo, "output.xml");

    Topology *topo2 = (Topology *)importFromXml("output.xml");

    for (auto c : c_orig) {
        int type = c->GetComponentType();
        int id = c->GetId();
        Component *c2 = topo2->GetDescendantById(id, type);
        if (c2 == NULL) {
            cout << "Component with id " << id << " and type " << type
                << " not found in imported topology" << endl;

            topo->Delete(true);
            topo2->Delete(true);

            // Delete output.xml
            //std::remove("output.xml");
            return 0;
        }
    }   
    cout << "Original topology:" << endl;
    topo->PrintSubtree();
    cout << "\nImported topology:" << endl;
    topo2->PrintSubtree();
    cout << "\nAll components found!" << endl;

    topo->Delete(true);
    topo2->Delete(true);

    // Delete output.xml
    std::remove("output.xml");
    return 0;
}

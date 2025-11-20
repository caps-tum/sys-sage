#include <iostream>
#include <filesystem>

#include "sys-sage.hpp"
#include "parsers/cccbench.hpp"

using std::cout;
using std::endl;

using namespace sys_sage;

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <hwloc xml path> <cccbench csv path>" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    std::string xmlPath;
    std::string cccPath;
    if(argc == 3){
        xmlPath = argv[1];
        cccPath = argv[2];
    }
    else{
        usage(argv[0]);
        return 1;
    }

    //create root Topology and one node
    Topology* topo = new Topology();
    Node* n = new Node(topo, 1);

    if(parseHwlocOutput(n, xmlPath) != 0) { //adds topo to a next node
        usage(argv[0]);
        return 1;
    }
    cout << "-- End parseHwlocOutput" << endl;

    cout << "Total num HW threads: " << topo->CountDescendantsByType(sys_sage::ComponentType::Thread) << endl;

    cout << "---------------- Printing the whole tree ----------------" << endl;
    topo->PrintSubtree();
    cout << "----------------                     ----------------" << endl;
    
    

    parseCccbenchOutput(n, cccPath);


    auto allcores = new std::vector<Component *>();
    topo->FindDescendantsByType(allcores, sys_sage::ComponentType::Core);    
    //auto allcores = topo->GetAllChildrenByType(SYS_SAGE_COMPONENT_CORE);

    for(auto c0 : *allcores)
        for(auto c1 : *allcores)
            cout << endl << c0->GetId() << " " << c1->GetId() << endl;

    cout << "---------------- Printing all DataPaths ----------------" << endl;
    n->PrintRelationsInSubtree();
    cout << "----------------                        ----------------" << endl;
    delete topo;
    delete n;
    return 0;
}

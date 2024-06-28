#include <iostream>
#include <filesystem>

#include "sys-sage.hpp"
#include "parsers/cccbench.hpp"

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <hwloc xml path> <cccbench csv path>" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    string xmlPath;
    const char *cccPath;
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

    cout << "Total num HW threads: " << topo->GetNumThreads() << endl;

    cout << "---------------- Printing the whole tree ----------------" << endl;
    topo->PrintSubtree(2);
    cout << "----------------                     ----------------" << endl;
    
    
    auto cccparser = new CccbenchParser(cccPath);
    cccparser->applyDataPaths(n);

    auto allcores = new vector<Component *>();
    topo->FindAllSubcomponentsByType(allcores, SYS_SAGE_COMPONENT_CORE);    
    //auto allcores = topo->GetAllChildrenByType(SYS_SAGE_COMPONENT_CORE);

    for(auto c0 : *allcores)
        for(auto c1 : *allcores)
            cout << endl << c0->GetId() << " " << c1->GetId() << endl;

    cout << "---------------- Printing all DataPaths ----------------" << endl;
    n->PrintAllDataPathsInSubtree();
    cout << "----------------                        ----------------" << endl;
    delete topo;
    delete n;
    return 0;
}

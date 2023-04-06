#include <iostream>

#include "sys-sage.hpp"
#include "musa_parser.hpp"


void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <musa system config file path>" << std::endl;
    std::cerr << "       or" << std::endl;
    std::cerr << "       " << argv0 << " (uses predefined paths which may be incorrect.)" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    string musaPath;
    if(argc < 2){
        std::string path_prefix(argv[0]);
        std::size_t found = path_prefix.find_last_of("/\\");
        path_prefix=path_prefix.substr(0,found) + "/";
        musaPath = path_prefix + "example_data/musa_custom_data_source.conf";
    }
    else if(argc == 2){
        musaPath = argv[1];
    }
    else{
        usage(argv[0]);
        return 1;
    }

    //create root Topology and one socket (Chip)
    Topology* topo = new Topology();
    Node* n = new Node(topo,0);
    Chip * socket = new Chip(/*parent*/ n, /*id*/ 0, /*name*/ "MUSA CPU", /*chip type*/ SYS_SAGE_CHIP_TYPE_CPU_SOCKET);

    cout << "-- Parsing MUSA custom data source from file " << musaPath << endl;
	if(parseMusa(socket, musaPath) != 0) {
        usage(argv[0]);
        return 1;
    }
    cout << "-- End parseMusa" << endl;
    cout << "Total num HW threads: " << topo->GetNumThreads() << endl;

    cout << "---------------- Printing the whole tree ----------------" << endl;
    topo->PrintSubtree(2);
    cout << "----------------                     ----------------" << endl;

    cout << "---------------- Printing all DataPaths ----------------" << endl;
    topo->PrintAllDataPathsInSubtree();
    cout << "----------------                        ----------------" << endl;

    string output_name = "custom_data_parser_musa_sample_output.xml";
    cout << "-------- Exporting as XML to " << output_name << " --------" << endl;
    exportToXml(topo, output_name);


	return 0;
}

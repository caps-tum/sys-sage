
#include <iostream>
#include <hwloc.h>

/*! \file */
/// @private
int hwloc_dump_xml(const char *filename);

using namespace std;

/**
Binary (entrypoint) for generating hwloc topology XML output (to current directory)
\n usage: ./hwloc-output [output_filename]
@param filename of the output file (default: tmp_hwloc.xml)
*/

int main(int argc, char* argv[])
{
    string xml_output;

    if (argc < 2) {
        xml_output = get_hwloc_topology_xml("tmp_hwloc.xml");
    }
    else {
        xml_output = get_hwloc_topology_xml(argv[1]);
    }

    if (!xml_output.empty()) {
        string filename = "hwloc_topology.xml";
        ofstream outfile(filename);
        outfile << xml_output;
        outfile.close();
        cout << "Hwloc XML output exported to " << filename << endl;
    }
    else {
        cerr << "Failed to generate hwloc topology XML output" << endl;
    }

    return 0;
}

string get_hwloc_topology_xml(const char* filename) {
    int err;
    unsigned long flags = 0; // don't show anything special
    hwloc_topology_t topology;

    err = hwloc_topology_init(&topology);
    if (err) {
        cerr << "hwloc: Failed to initialize" << endl;
        return "";
    }
    err = hwloc_topology_set_flags(topology, flags);
    if (err) {
        cerr << "hwloc: Failed to set flags" << endl;
        hwloc_topology_destroy(topology);
        return "";
    }
    err = hwloc_topology_load(topology);
    if (err) {
        cerr << "hwloc: Failed to load topology" << endl;
        hwloc_topology_destroy(topology);
        return "";
    }

    stringstream xml_output_stream;
    err = hwloc_topology_export_xml(topology, xml_output_stream, flags);
    if (err) {
        cerr << "hwloc: Failed to export xml" << endl;
        hwloc_topology_destroy(topology);
        return "";
    }

    hwloc_topology_destroy(topology);
    string xml_output = xml_output_stream.str();
    return xml_output;
}


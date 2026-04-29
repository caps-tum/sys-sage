import py_sys_sage as pysage 
import sys
import os

import sys
import os
import time

def usage(argv0):
    print("usage:", argv0, "[hwloc xml path] [xml output path/name]", file=sys.stderr)
    return

if __name__ == "__main__":
    xmlPath = None
    output_name = "sys-sage_cpu-frequency.xml"
    if len(sys.argv) < 2:
        path_prefix = os.path.dirname(sys.argv[0])
        xmlPath = os.path.join(path_prefix, "example_data", "skylake_hwloc.xml")
    elif len(sys.argv) == 2:
        xmlPath = sys.argv[1]
    elif len(sys.argv) == 3:
        xmlPath = sys.argv[1]
        output_name = sys.argv[2]
    else:
        usage(sys.argv[0])
        sys.exit(1)

    # create root Topology and one node
    topo = pysage.Topology()
    n = pysage.Node(topo, 1)

    print("-- Parsing Hwloc output from file", xmlPath)
    if pysage.parseHwlocOutput(n, xmlPath) != 0:  # adds topo to a next node
        usage(sys.argv[0])
        sys.exit(1)
    print("-- End parseHwlocOutput")

    print("-- Refresh frequency on core 1 (and do not store the timestamp). ")
    c1 = n.GetSubcomponentById(1, pysage.COMPONENT_CORE)
    if c1 is not None:
        c1.RefreshFreq(False)  #
        print("Frequency:", c1.freq)

    print("-- Refresh frequency on all cores of Node 1(and store the timestamp). ")
    # Frequency gets stored in attrib freq_history (value of type std::vector<std::tuple<long long=timestamp,double=frequency in MHz>>)
    repeat = 10
    for i in range(repeat):
        n.RefreshCpuCoreFrequency(True)
        time.sleep(0.1)  # 100 ms

    print("-- Print out frequency history on core 1 of Node 1. ")
    fh = c1.freq_history
    for ts, freq in fh.items():
        print("    ts:", ts, "frequency[MHz]:", freq)

    print("-- Export all information to xml", output_name)
    pysage.exportToXml(topo, output_name)
    
    topo.Delete(True)


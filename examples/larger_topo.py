import py_sys_sage as pysage 
import sys
import os 

if __name__ == "__main__":
    path_prefix = sys.argv[0]
    path_prefix = os.path.dirname(path_prefix) + "/"
    xmlPath = path_prefix + "example_data/skylake_hwloc.xml"
    bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv"

    # create root Topology and one node
    topo = pysage.Topology()
    n = pysage.Node(topo, 1)
    for n_idx in range(8):
        n = pysage.Node(topo, n_idx)
        if pysage.parseHwlocOutput(n, xmlPath) != 0:
            print("error parsing hwloc in path " + xmlPath)
            sys.exit(1)
        if pysage.parseCapsNumaBenchmark(n, bwPath, ";") != 0:
            print("failed parsing caps-numa-benchmark in path " + bwPath)
            sys.exit(1)

    output_name = "sys-sage_sample_output.xml"
    print("Exporting as XML to " + output_name)
    pysage.exportToXml(topo, output_name)

import py_sys_sage as pysage
import sys
import os

def usage(argv0):
    print("usage: {} <hwloc xml path> <caps-numa-benchmark csv path>".format(argv0), file=sys.stderr)
    print("       or", file=sys.stderr)
    print("       {} (uses predefined paths which may be incorrect.)".format(argv0), file=sys.stderr)
    return

def main():
    if len(sys.argv) < 2:
        path_prefix = os.path.dirname(os.path.abspath(sys.argv[0]))
        xmlPath = os.path.join(path_prefix, "example_data", "skylake_hwloc.xml")
        bwPath = os.path.join(path_prefix, "example_data", "skylake_caps_numa_benchmark.csv")
    elif len(sys.argv) == 3:
        xmlPath = sys.argv[1]
        bwPath = sys.argv[2]
    else:
        usage(sys.argv[0])
        return 1

    #create root Topology and one node
    topo = pysage.Topology()
    n = pysage.Node(topo, 1)

    print("-- Parsing Hwloc output from file", xmlPath)
    if pysage.parseHwlocOutput(n, xmlPath) != 0: #adds topo to a next node
        usage(sys.argv[0])
        return 1
    print("-- End parseHwlocOutput")

    print("Total num HW threads:", topo.CountAllSubcomponentsByType(pysage.COMPONENT_THREAD))
    print("---------------- Printing the whole tree ----------------")
    topo.PrintSubtree()
    print("----------------                     ----------------")

    print("-- Parsing CapsNumaBenchmark output from file", bwPath)
    if pysage.parseCapsNumaBenchmark(n, bwPath, ";") != 0:
        print("failed parsing caps-numa-benchmark")
        usage(sys.argv[0])
        return 1
    print("-- End parseCapsNumaBenchmark")

    print("---------------- Printing all DataPaths ----------------")
    n.PrintAllDataPathsInSubtree()
    print("----------------                        ----------------")

    output_name = "sys-sage_sample_output.xml"
    print("-------- Exporting as XML to", output_name, "--------")
    pysage.exportToXml(topo, output_name)

    print("--------Clearing up the tree--------")
    topo.Delete(True)

    return 0

if __name__ == "__main__":
    main()
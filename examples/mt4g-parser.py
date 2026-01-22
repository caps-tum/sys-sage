import sys_sage as syge 
import sys
import os
def usage(argv0):
    print("usage: {} <mt4g output path>".format(argv0), file=sys.stderr)
    print("       or", file=sys.stderr)
    print("       {} (uses predefined paths which may be incorrect.)".format(argv0), file=sys.stderr)
    return

if __name__ == "__main__":
    gpuTopoPath = None
    if len(sys.argv) < 2:
        path_prefix = os.path.dirname(os.path.abspath(sys.argv[0]))
        gpuTopoPath = os.path.join(path_prefix, "example_data", "ampere_gpu_topo.csv")
    elif len(sys.argv) == 2:
        gpuTopoPath = sys.argv[1]
    else:
        usage(sys.argv[0])
        sys.exit(1)

    # create root Topology and one node
    topo = syge.Topology()
    n = syge.Node(topo, 1)

    print("-- Parsing mt4g output from file", gpuTopoPath)
    if syge.ParseMt4g_v0_1(n, gpuTopoPath, 0, ";") != 0:  # adds topo to a next node
        print("failed parsing mt4g output")
        usage(sys.argv[0])
        sys.exit(1)
    print("-- End parseGpuTopo")

    print("Total num GPU cores:", topo.CountAllSubcomponentsByType(syge.COMPONENT_THREAD))

    output_name = "sys-sage_gpu_sample_output.xml"
    print("-------- Exporting as XML to", output_name, "--------")

    syge.exportToXml(topo, output_name)

    topo.Delete(True)


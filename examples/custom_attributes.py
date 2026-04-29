import py_sys_sage as pysage
import sys
import os

class My_core_attributes:
    def __init__(self, t, f):
        self.temperature = t
        self.frequency = f

# define your own print function for xml output
# key is the attrib key 
# value is the attrib value
# must return string value or none
def print_my_attribs(key : str, value : any) -> str:
    return str(value) if key in ("codename", "info", "rack_no") else None

# define your own complex print function for xml output
# key is the attrib key
# value is the attrib value
# must return xml string
def print_my_complex_attribs(key : str, value : any) -> str:
    if key == "my_core_info":
        return f"<root><Attribute name=\"{key}\"><my_core_info temperature=\"{value.temperature}\" temp_unit=\"C\" frequency=\"{value.frequency}\" freq_unit=\"Hz\"/></Attribute></root>"
    return None

def usage (argv0):
    print("usage: {} <hwloc xml path> <caps-numa-benchmark csv path>".format(argv0), file=sys.stderr)
    print("       or", file=sys.stderr)
    print("       {} (uses predefined paths which may be incorrect.)".format(argv0), file=sys.stderr)
    return

if __name__ == "__main__":
    xmlPath = None
    bwPath = None
    if len(sys.argv) < 2:
        path_prefix = sys.argv[0]
        path_prefix = os.path.dirname(path_prefix)
        xmlPath = os.path.join(path_prefix, "example_data/skylake_hwloc.xml")
        bwPath = os.path.join(path_prefix, "example_data/skylake_caps_numa_benchmark.csv")
    elif len(sys.argv) == 3:
        xmlPath = sys.argv[1]
        bwPath = sys.argv[2]
    else:
        usage(sys.argv[0])
        sys.exit(1)

    # create root Topology and one node
    topo = pysage.Topology()
    n = pysage.Node(topo, 1)

    print("-- Parsing Hwloc output from file " + xmlPath)
    if pysage.parseHwlocOutput(n, xmlPath) != 0:  
        usage(sys.argv[0])
        sys.exit(1)
    print("-- End parseHwlocOutput")
    print("-- Parsing CapsNumaBenchmark output from file " + bwPath)
    if pysage.parseCapsNumaBenchmark(n, bwPath, ";") != 0:
        print("failed parsing caps-numa-benchmark")
        usage(sys.argv[0])
        sys.exit(1)
    print("-- End parseCapsNumaBenchmark")

    # let's add a few custom attributes
    codename = "marsupial"
    r = 15
    n.codename = codename
    n.rack_no = r
    n.unknown_will_not_be_printed = xmlPath

    c1_attrib = My_core_attributes(38.222, 2000000000)
    c1 = n.GetSubcomponentById(1, pysage.COMPONENT_CORE)
    if c1 is not None:
        c1.my_core_info = c1_attrib

    c4_attrib = My_core_attributes(44.1, 1500000000)
    c4 = n.GetSubcomponentById(4, pysage.COMPONENT_CORE)
    if c4 is not None:
        c4.my_core_info = c4_attrib

    benchmark_info = "measured with no load on 07.07."
    n2 = n.GetSubcomponentById(2, pysage.COMPONENT_NUMA)
    if n2 is not None:
        dp = n2.GetDataPaths(pysage.DATAPATH_INCOMING)[0]
        if dp is not None:
            dp.info = benchmark_info
        dp = n2.GetDataPaths(pysage.DATAPATH_INCOMING)[2]
        if dp is not None:
            dp.info = benchmark_info

    # export to xml
    output_name = "sys-sage_custom_attributes.xml"
    print("-- Export all information to xml " + output_name)
    pysage.exportToXml(topo, output_name, print_my_attribs, print_my_complex_attribs)

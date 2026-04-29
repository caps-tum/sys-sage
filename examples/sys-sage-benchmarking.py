#TODO Fix this bullshit

import py_sys_sage as pysage 
import sys 
import os  
import timeit


#this file benchmarks and prints out performance information about basic operations with sys-sage
if __name__ == "__main__":
    path_prefix = sys.argv[0]
    path_prefix = os.path.dirname(path_prefix) + "/"
    xmlPath = path_prefix + "example_data/skylake_hwloc.xml"
    bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv"
    mt4gPath = path_prefix + "example_data/ampere_gpu_topo.csv"

    t = pysage.Topology()
    n = pysage.Node(t, 1)


    #time export sys_sage representation
    def search_simple(k, v):
        if "benchmark" in k:
            return str(v)+" success"
        return None
    def search_complex(k, v):
        if "complex" in k:
            return "<sys-sage><Attribute key=\"" + str(k) + "\" value=\"" + str(v) + "\"/></sys-sage>"
        return None
    
    n["benchmark"] = 1000
    n["complex"] = 1000
    time_export = timeit.timeit(lambda: pysage.exportToXml(t, "test.xml", None, None), number=100)
    print("time export to xml: ", time_export*10000000)
    
    #time import sys_sage respresentation
    def import_search_simple(x):
        if "benchmark" in x:
            y = x.split("value=\"")[-1]
            return y.rstrip("\"/>")
        return None
    def import_search_complex(x,c):
        if "complex" in x:
            y = x.split("value=\"")[-1]
            val = y.rstrip("\"/>")
            c["complex"] = int(val)
            return 1
        return 0
    time_import = timeit.timeit(lambda: pysage.importFromXml("test.xml", None, None), number=100)
    print("time import from xml: ", time_import*10000000)

    #time create new component
    time_createNewComponent = min(timeit.repeat(lambda: pysage.Node(t, 1), number=1000, repeat=3))
    print("time create new component: ",time_createNewComponent*1000000)
    
    #time hwloc parsing
    
    time_hwloc = timeit.timeit(lambda: pysage.parseHwlocOutput(n, xmlPath), number=1)
    print("time hwloc parsing: ", time_hwloc*1000000000)
    

    # time get a vector with all components (of hwloc parsing)
    
    time_getAllComponents = min(timeit.repeat(lambda: n.GetComponentsInSubtree(), number=100, repeat=3))
    print("time for Get hwloc SubtreeNodeList:", time_getAllComponents*10000000)
    
    # time caps-numa-benchmark parsing
    
    time_parseCapsNumaBenchmark = timeit.timeit(lambda: pysage.parseCapsNumaBenchmark(n, bwPath), number=1)
    print("time for parseCapsNumaBenchmark: ", time_parseCapsNumaBenchmark*1000000000)
    # n = pysage.Node(2)
    # pysage.parseHwlocOutput(n, xmlPath)
    # pysage.parseCapsNumaBenchmark(n, bwPath)
    
    numa = n.GetSubcomponentById(0, pysage.COMPONENT_NUMA)
    max_bw_comp = None
    def max_bw(m):
        max_bw = 0
        dps = m.GetDataPaths(pysage.DATAPATH_OUTGOING)
        for dp in dps:
            if dp.bandwidth > max_bw:
                max_bw = dp.bandwidth
                max_bw_comp = dp.target
        return max_bw_comp
    
    #time get numa max_bw
    time_numa_max_bw = min(timeit.repeat(lambda: max_bw(numa), number = 100, repeat = 3 ))
    print("time for numa max bw: ", time_numa_max_bw*10000000)
    
    
    gpu = pysage.Chip(n,100,"GPU")
    
    #time mt4g parsing
    time_parse_mt4g = timeit.timeit(lambda: pysage.parseMt4gTopo(gpu, mt4gPath, ";"), number=1)
    print("time for parsing mt4g: ",time_parse_mt4g*1000000000)
    
    #time get mt4g subtree node list
    time_get_mt4g_subtree = min(timeit.repeat(lambda: gpu.GetComponentsInSubtree(), number=100, repeat=3))
    print("time for get mt4g subtree: ",time_get_mt4g_subtree*10000000)
    
    
    #time get all components list
    time_get_all_componentes = min(timeit.repeat(lambda: t.GetComponentsInSubtree(), number=100, repeat=3))
    print("time for get all components: ",time_get_all_componentes*10000000)
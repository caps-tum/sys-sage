import py_sys_sage as pysage 
import sys
import os
def usage(argv0):
    print(f"usage: {argv0} <hwloc xml path> <cccbench csv path>", file=sys.stderr)

def main():
    if len(sys.argv) != 3:
        usage(sys.argv[0])
        return 1

    xmlPath = sys.argv[1]
    cccPath = sys.argv[2]

    # create root Topology and one node
    topo = pysage.Topology()
    n = pysage.Node(topo, 1)

    if pysage.parseHwlocOutput(n, xmlPath) != 0:  # adds topo to a next node
        usage(sys.argv[0])
        return 1
    print("-- End parseHwlocOutput")

    print("Total num HW threads:", topo.CountAllSubcomponentsByType(pysage.COMPONENT_THREAD))

    print("---------------- Printing the whole tree ----------------")
    topo.PrintSubtree(2)
    print("----------------                     ----------------")

    pysage.parseCccbenchOutput(n, cccPath)

    allcores = []
    topo.GetAllSubcomponentsByType(allcores, pysage.COMPONENT_CORE)

    for c0 in allcores:
        for c1 in allcores:
            print(f"\n{c0.GetId()} {c1.GetId()}")

    print("---------------- Printing all DataPaths ----------------")
    n.PrintAllDataPathsInSubtree()
    print("----------------                        ----------------")

    return 0

if __name__ == "__main__":
    main()


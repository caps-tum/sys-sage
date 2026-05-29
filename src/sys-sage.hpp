#ifndef SYS_SAGE_HPP
#define SYS_SAGE_HPP

//includes internal headers

#include <sys-sage/Component.hpp>
#include <sys-sage/Topology.hpp>
#include <sys-sage/Node.hpp>
#include <sys-sage/Storage.hpp>
#include <sys-sage/Chip.hpp>
#include <sys-sage/Subdivision.hpp>
#include <sys-sage/Numa.hpp>
#include <sys-sage/Memory.hpp>
#include <sys-sage/Cache.hpp>
#include <sys-sage/Core.hpp>
#include <sys-sage/Thread.hpp>
#include <sys-sage/QuantumBackend.hpp>
#include <sys-sage/AtomSite.hpp>
#include <sys-sage/Qubit.hpp>

#include <sys-sage/Relation.hpp>
#include <sys-sage/DataPath.hpp>
#include <sys-sage/QuantumGate.hpp>
#include <sys-sage/CouplingMap.hpp>

#include <sys-sage/attribute.hpp>
#include <sys-sage/json_serialization.hpp>
#include <sys-sage/xml_dump.hpp>
#include <sys-sage/xml_load.hpp>

#include <sys-sage/parsers/hwloc.hpp>
#include <sys-sage/parsers/caps-numa-benchmark.hpp>
#include <sys-sage/parsers/mt4g.hpp>
#include <sys-sage/parsers/cccbench.hpp>
#include <sys-sage/parsers/iqm-parser.hpp>

#include <sys-sage/external_interfaces/ss_papi.hpp>
#include <sys-sage/external_interfaces/qdmi-parser.hpp>

#endif

#include "sys-sage.hpp"

#include <boost/ut.hpp>
#include <string_view>

using namespace boost::ut;
using namespace sys_sage;
using namespace std::string_view_literals;

static suite<"hwloc"> _ = []
{
    Topology topo;
    Node node{&topo};
    expect(that % (0 == parseHwlocOutput(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_hwloc.xml")) >> fatal);

    for (const auto &[type, count] : std::vector{
             std::tuple{ComponentType::Chip, 2},
             std::tuple{ComponentType::Numa, 4},
             std::tuple{ComponentType::Cache, 50},
             std::tuple{ComponentType::Core, 24},
             std::tuple{ComponentType::Thread, 24},
         })
    {
        std::vector<Component *> components;
        topo.FindDescendantsByType(&components, type);
        expect(that % _u(count) == components.size());
    }

    auto chip = dynamic_cast<Chip *>(node.GetChildByType(ComponentType::Chip));
    expect(that % (chip != nullptr) >> fatal);
    expect(that % "GenuineIntel"sv == chip->GetVendor());
    expect(that % "Intel(R) Xeon(R) Silver 4116 CPU @ 2.10GHz"sv == chip->GetModel());

    auto cacheL3 = dynamic_cast<Cache *>(chip->GetChildByType(ComponentType::Cache));
    expect(that % (cacheL3 != nullptr) >> fatal);
    expect(that % 3 == cacheL3->GetCacheLevel());
    expect(that % 17301504 == cacheL3->GetCacheSize());
    expect(that % 11 == cacheL3->GetCacheAssociativityWays());
    expect(that % 64 == cacheL3->GetCacheLineSize());

    auto numa = dynamic_cast<Numa *>(cacheL3->GetChildByType(ComponentType::Numa));
    expect(that % (numa != nullptr) >> fatal);

    auto cacheL2 = dynamic_cast<Cache *>(numa->GetChildByType(ComponentType::Cache));
    expect(that % (cacheL2 != nullptr) >> fatal);
    expect(that % 2 == cacheL2->GetCacheLevel());
    expect(that % 1048576 == cacheL2->GetCacheSize());
    expect(that % 16 == cacheL2->GetCacheAssociativityWays());
    expect(that % 64 == cacheL2->GetCacheLineSize());

    auto cacheL1 = dynamic_cast<Cache *>(cacheL2->GetChildByType(ComponentType::Cache));
    expect(that % (cacheL1 != nullptr) >> fatal);
    expect(that % 1 == cacheL1->GetCacheLevel());
    expect(that % 32768 == cacheL1->GetCacheSize());
    expect(that % 8 == cacheL1->GetCacheAssociativityWays());
    expect(that % 64 == cacheL1->GetCacheLineSize());

    auto core = dynamic_cast<Core *>(cacheL1->GetChildByType(ComponentType::Core));
    expect(that % (core != nullptr) >> fatal);

    auto thread = dynamic_cast<Thread *>(core->GetChildByType(ComponentType::Thread));
    expect(that % (thread != nullptr) >> fatal);
};

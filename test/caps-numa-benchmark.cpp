#include <boost/ut.hpp>
#include <string_view>
#include <algorithm>

#include "sys-sage.hpp"

using namespace boost::ut;
using namespace sys_sage;
using namespace std::string_view_literals;

static suite<"caps-numa-benchmark"> _ = []
{
    Topology topo;
    Node node{&topo};

    expect(that % (0 == parseHwlocOutput(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_hwloc.xml")) >> fatal)
        << "Parse hwloc XML file";

    expect(that % (0 == parseCapsNumaBenchmark(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_caps_numa_benchmark.csv")) >> fatal)
        << "Parse benchmark CSV file";

    std::vector<Component *> numas;
    node.FindDescendantsByType(&numas, ComponentType::Numa);
    expect(that % (4 == numas.size()) >> fatal);

    "Number, type, and orientation of data paths"_test = [&]
    {
        for (const auto &numa : numas)
        {
            expect(that % (4 == numa->FindDataPaths(DataPathType::Any, DataPathDirection::Incoming).size()) >> fatal);
            for (const auto &dp : numa->FindDataPaths(DataPathType::Any, DataPathDirection::Incoming))
            {
                expect(that % DataPathType::Datatransfer == dp->GetDataPathType());
                expect(that % DataPathOrientation::Oriented == dp->GetOrientation());
            }

            expect(that % (4 == numa->FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing).size()) >> fatal);
            for (const auto &dp : numa->FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing))
            {
                expect(that % DataPathType::Datatransfer == dp->GetDataPathType());
                expect(that % DataPathOrientation::Oriented == dp->GetOrientation());
            }
        }
    };

    "Anti-symmetry of data paths"_test = [&]
    {
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t k = 0; k < 4; ++k)
            {
                auto dp1 = (numas[i]->FindDataPaths(DataPathType::Any, DataPathDirection::Incoming))[k];
                auto dp2 = (numas[k]->FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing))[i];
                expect(that % dp1->GetBandwidth() == dp2->GetBandwidth());
                expect(that % dp1->GetLatency() == dp2->GetLatency());
            }
        }
    };

    "Bandwidth and latency of data paths"_test = [&]
    {
        auto dp = [&numas](size_t i, size_t k)
        {
            return (numas[i]->FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing))[k];
        };

        expect(that % 8621 == dp(0, 0)->GetBandwidth());
        expect(that % 8502 == dp(1, 0)->GetBandwidth());
        expect(that % 6467 == dp(2, 0)->GetBandwidth());
        expect(that % 6476 == dp(3, 0)->GetBandwidth());
        expect(that % 244 == dp(0, 0)->GetLatency());
        expect(that % 195 == dp(1, 0)->GetLatency());
        expect(that % 307 == dp(2, 0)->GetLatency());
        expect(that % 313 == dp(3, 0)->GetLatency());

        expect(that % 8237 == dp(0, 1)->GetBandwidth());
        expect(that % 8663 == dp(1, 1)->GetBandwidth());
        expect(that % 6291 == dp(2, 1)->GetBandwidth());
        expect(that % 6267 == dp(3, 1)->GetBandwidth());
        expect(that % 203 == dp(0, 1)->GetLatency());
        expect(that % 237 == dp(1, 1)->GetLatency());
        expect(that % 315 == dp(2, 1)->GetLatency());
        expect(that % 319 == dp(3, 1)->GetLatency());

        expect(that % 6439 == dp(0, 2)->GetBandwidth());
        expect(that % 6609 == dp(1, 2)->GetBandwidth());
        expect(that % 8539 == dp(2, 2)->GetBandwidth());
        expect(that % 8412 == dp(3, 2)->GetBandwidth());
        expect(that % 302 == dp(0, 2)->GetLatency());
        expect(that % 313 == dp(1, 2)->GetLatency());
        expect(that % 237 == dp(2, 2)->GetLatency());
        expect(that % 210 == dp(3, 2)->GetLatency());

        expect(that % 6315 == dp(0, 3)->GetBandwidth());
        expect(that % 6324 == dp(1, 3)->GetBandwidth());
        expect(that % 8177 == dp(2, 3)->GetBandwidth());
        expect(that % 8466 == dp(3, 3)->GetBandwidth());
        expect(that % 309 == dp(0, 3)->GetLatency());
        expect(that % 316 == dp(1, 3)->GetLatency());
        expect(that % 211 == dp(2, 3)->GetLatency());
        expect(that % 246 == dp(3, 3)->GetLatency());
    };
};

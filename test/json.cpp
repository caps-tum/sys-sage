#include "sys-sage.hpp"
#include <boost/ut.hpp>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string_view>

namespace ut = boost::ut;
using namespace sys_sage;
using namespace std::string_view_literals;
using json = nlohmann::ordered_json;

static ut::suite<"json"> _ = []
{
    ut::test("correct topology") = []
    {
        constexpr long long numaSizes[4] = { 24904634368, 25365467136, 25365463040, 25364619264 };
        int numaSizeIdx = 0;

        Node node;
        parseHwlocOutput(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_hwloc.xml");

        const json topoJson = json(&node);

        ut::expect(ut::that % topoJson["type"].get<std::string>() == "Node"sv);
        ut::expect(ut::that % topoJson["children"].size() == 2U);

        for (const json &chip : topoJson["children"]) {
            ut::expect(ut::that % chip["type"].get<std::string>() == "Chip"sv);
            ut::expect(ut::that % chip["vendor"].get<std::string>() == "GenuineIntel"sv);
            ut::expect(ut::that % chip["model"].get<std::string>() == "Intel(R) Xeon(R) Silver 4116 CPU @ 2.10GHz"sv);
            ut::expect(ut::that % chip["chipCategory"].get<ChipCategory::type>() == ChipCategory::CpuSocket);
            ut::expect(ut::that % chip["children"].size() == 1U);

            const json &l3Cache = chip["children"][0];
            ut::expect(ut::that % l3Cache["type"].get<std::string>() == "Cache"sv);
            ut::expect(ut::that % l3Cache["cacheType"].get<std::string>() == "3"sv);
            ut::expect(ut::that % l3Cache["cacheSize"].get<long long>() == 17301504);
            ut::expect(ut::that % l3Cache["cacheLineSize"].get<int>() == 64);
            ut::expect(ut::that % l3Cache["cacheAssociativityWays"].get<int>() == 11);
            ut::expect(ut::that % l3Cache["children"].size() == 2U);

            for (const json &numa : l3Cache["children"]) {
                ut::expect(ut::that % numa["type"].get<std::string>() == "NUMA"sv);
                ut::expect(ut::that % numa["size"].get<long long>() == numaSizes[numaSizeIdx++]);
                ut::expect(ut::that % numa["subdivisionCategory"].get<SubdivisionCategory::type>() == SubdivisionCategory::None);
                ut::expect(ut::that % numa["children"].size() == 6U);

                for (const json &l2Cache : numa["children"]) {
                    ut::expect(ut::that % l2Cache["type"].get<std::string>() == "Cache"sv);
                    ut::expect(ut::that % l2Cache["cacheType"].get<std::string>() == "2"sv);
                    ut::expect(ut::that % l2Cache["cacheSize"].get<long long>() == 1048576);
                    ut::expect(ut::that % l2Cache["cacheLineSize"].get<int>() == 64);
                    ut::expect(ut::that % l2Cache["cacheAssociativityWays"].get<int>() == 16);
                    ut::expect(ut::that % l2Cache["children"].size() == 1U);

                    const json &l1Cache = l2Cache["children"][0];
                    ut::expect(ut::that % l1Cache["type"].get<std::string>() == "Cache"sv);
                    ut::expect(ut::that % l1Cache["cacheType"].get<std::string>() == "1"sv);
                    ut::expect(ut::that % l1Cache["cacheSize"].get<long long>() == 32768);
                    ut::expect(ut::that % l1Cache["cacheLineSize"].get<int>() == 64);
                    ut::expect(ut::that % l1Cache["cacheAssociativityWays"].get<int>() == 8);
                    ut::expect(ut::that % l1Cache["children"].size() == 1U);

                    const json &core = l1Cache["children"][0];
                    ut::expect(ut::that % core["type"].get<std::string>() == "Core"sv);
                    ut::expect(ut::that % core["children"].size() == 1U);

                    const json &hwThread = core["children"][0];
                    ut::expect(ut::that % hwThread["type"].get<std::string>() == "HW_Thread"sv);
                }
            }
        }
    };

    ut::test("export -> import") = []
    {
        Node *node1 = new Node;
        parseHwlocOutput(node1, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_hwloc.xml");
        ut::expect(ut::that % DumpJson(node1, "topo.json") == 0) ;

        Component *node2 = LoadJson("topo.json");
        ut::expect(ut::that % node2 != nullptr) ;

        json topo1 = json(node1);
        json topo2 = json(node2);

        ut::expect(ut::that % topo1["type"].get<std::string>() == topo2["type"].get<std::string>());
        ut::expect(ut::that % topo1["children"].size() == 2U);
        ut::expect(ut::that % topo2["children"].size() == 2U);

        for (size_t sChip = 0; sChip < topo1["children"].size(); sChip++) {
            const json &chip1 = topo1["children"][sChip];
            const json &chip2 = topo2["children"][sChip];

            ut::expect(ut::that % chip1["type"].get<std::string>() == chip2["type"].get<std::string>());
            ut::expect(ut::that % chip1["vendor"].get<std::string>() == chip2["vendor"].get<std::string>());
            ut::expect(ut::that % chip1["model"].get<std::string>() == chip2["model"].get<std::string>());
            ut::expect(ut::that % chip1["chipCategory"].get<ChipCategory::type>() == chip2["chipCategory"].get<ChipCategory::type>());
            ut::expect(ut::that % chip1["children"].size() == 1U);
            ut::expect(ut::that % chip2["children"].size() == 1U);

            const json &l3Cache1 = chip1["children"][0];
            const json &l3Cache2 = chip2["children"][0];

            ut::expect(ut::that % l3Cache1["type"].get<std::string>() == l3Cache2["type"].get<std::string>());
            ut::expect(ut::that % l3Cache1["cacheType"].get<std::string>() == l3Cache2["cacheType"].get<std::string>());
            ut::expect(ut::that % l3Cache1["cacheSize"].get<long long>() == l3Cache2["cacheSize"].get<long long>());
            ut::expect(ut::that % l3Cache1["cacheLineSize"].get<int>() == l3Cache2["cacheLineSize"].get<int>());
            ut::expect(ut::that % l3Cache1["cacheAssociativityWays"].get<int>() == l3Cache2["cacheAssociativityWays"].get<int>());
            ut::expect(ut::that % l3Cache1["children"].size() == 2U);
            ut::expect(ut::that % l3Cache2["children"].size() == 2U);

            for (size_t sNuma = 0; sNuma < l3Cache1["children"].size(); sNuma++) {
                const json &numa1 = l3Cache1["children"][sNuma];
                const json &numa2 = l3Cache2["children"][sNuma];

                ut::expect(ut::that % numa1["type"].get<std::string>() == numa2["type"].get<std::string>());
                ut::expect(ut::that % numa1["size"].get<long long>() == numa2["size"].get<long long>());
                ut::expect(ut::that % numa1["subdivisionCategory"].get<SubdivisionCategory::type>() == numa2["subdivisionCategory"].get<SubdivisionCategory::type>());
                ut::expect(ut::that % numa1["children"].size() == 6U);
                ut::expect(ut::that % numa2["children"].size() == 6U);

                for (size_t sL2Cache = 0; sL2Cache < numa1["children"].size(); sL2Cache++) {
                    const json &l2Cache1 = numa1["children"][sL2Cache];
                    const json &l2Cache2 = numa2["children"][sL2Cache];

                    ut::expect(ut::that % l2Cache1["type"].get<std::string>() == l2Cache2["type"].get<std::string>());
                    ut::expect(ut::that % l2Cache1["cacheType"].get<std::string>() == l2Cache2["cacheType"].get<std::string>());
                    ut::expect(ut::that % l2Cache1["cacheSize"].get<long long>() == l2Cache2["cacheSize"].get<long long>());
                    ut::expect(ut::that % l2Cache1["cacheLineSize"].get<int>() == l2Cache2["cacheLineSize"].get<int>());
                    ut::expect(ut::that % l2Cache1["cacheAssociativityWays"].get<int>() == l2Cache2["cacheAssociativityWays"].get<int>());
                    ut::expect(ut::that % l2Cache1["children"].size() == 1U);
                    ut::expect(ut::that % l2Cache2["children"].size() == 1U);

                    const json &l1Cache1 = l2Cache1["children"][0];
                    const json &l1Cache2 = l2Cache2["children"][0];

                    ut::expect(ut::that % l1Cache1["type"].get<std::string>() == l1Cache2["type"].get<std::string>());
                    ut::expect(ut::that % l1Cache1["cacheType"].get<std::string>() == l1Cache2["cacheType"].get<std::string>());
                    ut::expect(ut::that % l1Cache1["cacheSize"].get<long long>() == l1Cache2["cacheSize"].get<long long>());
                    ut::expect(ut::that % l1Cache1["cacheLineSize"].get<int>() == l1Cache2["cacheLineSize"].get<int>());
                    ut::expect(ut::that % l1Cache1["cacheAssociativityWays"].get<int>() == l1Cache2["cacheAssociativityWays"].get<int>());
                    ut::expect(ut::that % l1Cache1["children"].size() == 1U);
                    ut::expect(ut::that % l1Cache2["children"].size() == 1U);

                    const json &core1 = l1Cache1["children"][0];
                    const json &core2 = l1Cache2["children"][0];

                    ut::expect(ut::that % core1["type"].get<std::string>() == core2["type"].get<std::string>());
                    ut::expect(ut::that % core1["children"].size() == 1U);
                    ut::expect(ut::that % core2["children"].size() == 1U);

                    const json &hwThread1 = core1["children"][0];
                    const json &hwThread2 = core2["children"][0];

                    ut::expect(ut::that % hwThread1["type"].get<std::string>() == hwThread2["type"].get<std::string>());
                }
            }
        }

        std::remove("topo.json");

        delete node1;
        delete node2;
    };

    ut::test("relations") = []
    {
        Component comp1;
        Component comp2;

        Relation rel;
        rel.AddComponent(&comp1);
        rel.AddComponent(&comp2);

        json relJson = json(&rel);
        ut::expect(ut::that % relJson["type"].get<std::string>() == "Relation"sv);
        ut::expect(ut::that % relJson["category"].get<RelationCategory::type>() == RelationCategory::Default);
        ut::expect(ut::that % relJson["ordered"].get<bool>() == true);

        ut::expect(ut::that % relJson["components"].size() == 2U);
        ut::expect(ut::that % relJson["components"][0].get<uintptr_t>() == reinterpret_cast<uintptr_t>(&comp1));
        ut::expect(ut::that % relJson["components"][1].get<uintptr_t>() == reinterpret_cast<uintptr_t>(&comp2));
    };
};

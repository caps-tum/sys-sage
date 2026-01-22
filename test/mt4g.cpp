#include <boost/ut.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "sys-sage.hpp"

using namespace boost::ut;
using namespace sys_sage;
using namespace std::string_view_literals;

static suite<"mt4g"> _ = []
{
  "v0.1"_test = []
  {
    Topology topo;
    Chip gpu{&topo};
    expect(that % (0 == ParseMt4g_v0_1(&gpu, SYS_SAGE_TEST_RESOURCE_DIR "/pascal_gpu_topo.csv")) >> fatal);

    for (const auto &[type, count] : std::vector{
             std::tuple{ComponentType::Memory, 31},
             std::tuple{ComponentType::Subdivision, 30},
             std::tuple{ComponentType::Cache, 121},
             std::tuple{ComponentType::Thread, 3840},
         })
    {
        std::vector<Component *> components;
        topo.FindDescendantsByType(&components, type);
        expect(that % _u(count) == components.size());
    }

    expect(that % "Nvidia"sv == gpu.GetVendor());
    expect(that % "Quadro P6000"sv == gpu.GetModel());

    auto memory = dynamic_cast<Memory *>(gpu.GetChildByType(ComponentType::Memory));
    expect(that % (nullptr != memory) >> fatal);
    expect(that % 25637224578 == memory->GetSize());
    expect(that % 3840_u == memory->FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing).size());

    auto cacheL2 = dynamic_cast<Cache *>(memory->GetChildByType(ComponentType::Cache));
    expect(that % (nullptr != cacheL2) >> fatal);
    expect(that % 3145728 == cacheL2->GetCacheSize());
    expect(that % 32 == cacheL2->GetCacheLineSize());

    auto subdivision = dynamic_cast<Subdivision *>(cacheL2->GetChildByType(ComponentType::Subdivision));
    expect(that % (nullptr != subdivision) >> fatal);
    expect(that % SubdivisionType::GpuSM == subdivision->GetSubdivisionType());

    auto cacheL1 = dynamic_cast<Cache *>(subdivision->GetChildByType(ComponentType::Cache));
    expect(that % (nullptr != cacheL1) >> fatal);
    expect(that % 24588 == cacheL1->GetCacheSize());
    expect(that % 32 == cacheL1->GetCacheLineSize());

    auto thread = dynamic_cast<Thread *>(cacheL1->GetChildByType(ComponentType::Thread));
    expect(that % (nullptr != thread) >> fatal);
    //topo.Delete(true);
  };

  "NVIDIA"_test = []
  {
    const char *jsonPath = SYS_SAGE_TEST_RESOURCE_DIR "/NVIDIA_GeForce_RTX_2080_Ti.json";
    const std::string expectedVendor ("NVIDIA");
    const std::string expectedModel ("NVIDIA GeForce RTX 2080 Ti");
    const std::string cL1_5 ( "Constant L1.5" );
    const std::string cL1 ( "Constant L1" );
    const std::string l1 ( "L1+Read Only+Texture" );
    const std::string sharedMem ( "Shared Memory" );

    Node node;

    int rval = ParseMt4g(&node, jsonPath, 0);
    expect(that % rval == 0);

    expect(that % node.GetChildren().size() == 1U && node.GetChildren()[0]->GetComponentType() == ComponentType::Chip);
    auto gpu = static_cast<Chip *>( node.GetChildren()[0] );
    
    expect(that % gpu->GetVendor() == expectedVendor);
    expect(that % gpu->GetModel() == expectedModel);
    expect(that % *static_cast<long long *>(gpu->attrib["clockRate"]) == 1545000 * 1000);
    auto majorMinor = static_cast<std::pair<int, int> *>(gpu->attrib["computeCapability"]);
    expect(that % majorMinor->first == 7 && majorMinor->second == 5);

    expect(that % gpu->GetChildren().size() == 1U && gpu->GetChildren()[0]->GetComponentType() == ComponentType::Memory);
    auto mainMem = static_cast<Memory *>( gpu->GetChildren()[0] );

    expect(that % mainMem->GetChildren().size() == 1U && mainMem->GetChildren()[0]->GetComponentType() == ComponentType::Cache);
    auto l2Cache = static_cast<Cache *>( mainMem->GetChildren()[0] );
    expect(that % l2Cache->GetCacheLevel() == 2);

    size_t numMPs = l2Cache->GetChildren().size();
    expect(that % numMPs == 68U);

    for (auto mp : l2Cache->GetChildren()) {
      expect(that % mp->GetComponentType() == ComponentType::Subdivision);

      for (auto child : mp->GetChildren()) {
        ComponentType::type type = child->GetComponentType();
        expect(that % type == ComponentType::Cache || type == ComponentType::Memory);

        if (type == ComponentType::Memory) {
          expect(that % child->GetName() == sharedMem);
        } else {
          Cache *cache = static_cast<Cache *>( child );

          const std::string &name = cache->GetCacheName();
          expect(that % name == cL1_5 || name == l1);

          if (name == cL1_5) {
            expect(that % cache->GetChildren().size() == 1U && cache->GetChildren()[0]->GetComponentType() == ComponentType::Cache);
            Cache *cL1Cache = static_cast<Cache *>( cache->GetChildren()[0] );
            expect(that % cL1Cache->GetCacheName() == cL1);
          } else {
            expect(that % cache->GetChildren().size() == 64U);

            for (auto core : cache->GetChildren())
              expect(that % core->GetComponentType() == ComponentType::Thread);
          }
        }
      }
    }
  };

  "AMD"_test = []
  {
    const char *jsonPath = SYS_SAGE_TEST_RESOURCE_DIR "/AMD_Instinct_MI100.json";
    const std::string expectedVendor ("AMD");
    const std::string expectedModel ("AMD Instinct MI100");
    const std::string sL1 ("Scalar L1");
    const std::string constant ("Constant");
    const std::string sharedMem ( "Shared Memory" );
    const std::string l1 ( "L1" );

    Node node;

    int rval = ParseMt4g(&node, jsonPath, 0);
    expect(that % rval == 0);

    expect(that % node.GetChildren().size() == 1U && node.GetChildren()[0]->GetComponentType() == ComponentType::Chip);
    auto gpu = static_cast<Chip *>( node.GetChildren()[0] );
    
    expect(that % gpu->GetVendor() == expectedVendor);
    expect(that % gpu->GetModel() == expectedModel);
    expect(that % *static_cast<long long *>(gpu->attrib["clockRate"]) == 1502000 * 1000);
    auto majorMinor = static_cast<std::pair<int, int> *>(gpu->attrib["computeCapability"]);
    expect(that % majorMinor->first == 9 && majorMinor->second == 0);

    expect(that % gpu->GetChildren().size() == 1U && gpu->GetChildren()[0]->GetComponentType() == ComponentType::Memory);
    auto mainMem = static_cast<Memory *>( gpu->GetChildren()[0] );

    expect(that % mainMem->GetChildren().size() == 1U && mainMem->GetChildren()[0]->GetComponentType() == ComponentType::Cache);
    auto l2Cache = static_cast<Cache *>( mainMem->GetChildren()[0] );
    expect(that % l2Cache->GetCacheLevel() == 2);

    expect(that % l2Cache->GetChildren().size() == 48U);

    std::vector<Component *> mps;
    for (auto sL1Cache : l2Cache->GetChildren()) {
      expect(that % sL1Cache->GetComponentType() == ComponentType::Cache);
      expect(that % static_cast<Cache *>(sL1Cache)->GetCacheName() == sL1);

      for (auto child : sL1Cache->GetChildren())
        expect(that % child->GetComponentType() == ComponentType::Subdivision);
      mps.insert(mps.end(), sL1Cache->GetChildren().begin(), sL1Cache->GetChildren().end());
    }

    expect(that % mps.size() == 120U);

    for (auto mp : mps) {
      for (auto child : mp->GetChildren()) {
        ComponentType::type type = child->GetComponentType();
        expect(that % type == ComponentType::Cache || type == ComponentType::Memory);

        if (type == ComponentType::Memory) {
          expect(that % child->GetName() == sharedMem);
        } else {
          Cache *cache = static_cast<Cache *>( child );

          const std::string &name = cache->GetCacheName();
          expect(that % name == constant || name == l1);

          if (name == l1) {
            expect(that % cache->GetChildren().size() == 64U);

            for (auto core : cache->GetChildren())
              expect(that % core->GetComponentType() == ComponentType::Thread);
          }
        }
      }
    }
  };
};

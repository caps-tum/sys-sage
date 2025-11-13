#include "mt4g.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace sys_sage;
using json = nlohmann::json;

#define kHz_to_Hz(kHz) (kHz) * 1000
#define GiBs_to_Bs(GiBs) (GiBs) * 1073741824

static void ParseGeneral(const json &general, Chip *gpu)
{
  gpu->SetModel(general["name"].get<std::string>());
  gpu->SetVendor(general["vendor"].get<std::string>());

  gpu->attrib["computeCapability"] = reinterpret_cast<void *>(
    new std::pair<int, int> {
          general["computeCapability"]["major"].get<int>(),
          general["computeCapability"]["minor"].get<int>()
        }
  );

  gpu->attrib["clockRate"] = reinterpret_cast<void *>(
    new double( kHz_to_Hz(general["clockRate"]["value"].get<double>()) )
  );
}

static std::tuple<int, int> ParseCompute(const json &compute, Chip *gpu)
{
  auto multiProcessorCount = new int( compute["multiProcessorCount"].get<int>() );
  gpu->attrib["multiProcessorCount"] = reinterpret_cast<void *>( multiProcessorCount );

  auto numberOfCoresPerMultiProcessor = new int( compute["numberOfCoresPerMultiProcessor"].get<int>() );
  gpu->attrib["numberOfCoresPerMultiProcessor"] = reinterpret_cast<void *>( numberOfCoresPerMultiProcessor );

  gpu->attrib["maxThreadsPerBlock"] = reinterpret_cast<void *>(
    new int( compute["maxThreadsPerBlock"].get<int>() )
  );
  gpu->attrib["warpSize"] = reinterpret_cast<void *>(
    new int( compute["warpSize"].get<int>() )
  );
  gpu->attrib["maxThreadsPerMultiProcessor"] = reinterpret_cast<void *>(
    new int( compute["maxThreadsPerMultiProcessor"].get<int>() )
  );
  gpu->attrib["maxBlocksPerMultiProcessor"] = reinterpret_cast<void *>(
    new int( compute["maxBlocksPerMultiProcessor"].get<int>() )
  );

  // TODO: maybe parse register info?

  if (auto it = compute.find("numXDCDs"); it != compute.end()) {
    gpu->attrib["numXDCDs"] = reinterpret_cast<void *>(
        new int( it->get<int>() )
    );
  }
  if (auto it = compute.find("computeUnitsPerDie"); it != compute.end()) {
    gpu->attrib["computeUnitsPerDie"] = reinterpret_cast<void *>(
        new int( it->get<int>() )
    );
  }
  if (auto it = compute.find("numSIMDsPerCu"); it != compute.end()) {
    gpu->attrib["numSIMDsPerCu"] = reinterpret_cast<void *>(
        new int( it->get<int>() )
    );
  }


  return { *multiProcessorCount, *numberOfCoresPerMultiProcessor };
}

static void ParseMainMemory(const json &mainJson, Chip *gpu,
                            std::vector<Thread *> &cores,
                            std::vector<Component *> &leafs)
{
  auto main = new Memory(gpu, 0, "GPU Main Memory",
                         mainJson["totalGlobalMem"]["value"].get<long long>());
  leafs.push_back(main);

  main->attrib["clockRate"] = reinterpret_cast<void *>(
    new double ( kHz_to_Hz(mainJson["memoryClockRate"]["value"].get<double>()) )
  );
  main->attrib["busWidth"] = reinterpret_cast<void *>(
    new int ( mainJson["memoryBusWidth"]["value"].get<int>() )
  );

  // the existence of `latency` implies the existence of `readBandwidth` and `writeBandwidth`
  if (auto it = mainJson.find("latency"); it != mainJson.end()) {
    double latency = (*it)["mean"].get<double>();
    double readBandwidth = GiBs_to_Bs( mainJson["readBandwidth"]["value"].get<double>() );
    double writeBandwidth = GiBs_to_Bs( mainJson["writeBandwidth"]["value"].get<double>() );

    for (auto core : cores) {
      auto dp = new DataPath(main, core, DataPathOrientation::Oriented,
                             DataPathType::Logical, -1, latency);
      dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
      dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
    }
  }
}

static void ParseL3Cache(const json &l3Json, std::vector<Thread *> &cores,
                         std::vector<Component *> &leafs)
{
  int amount = l3Json.value("amount", 1);
  int amountPerLeaf = amount / leafs.size();
  std::vector<Component *> l3Caches (amount);

  long long size = l3Json["size"]["value"].get<long long>();

  int lineSize = -1;
  if (auto it = l3Json.find("lineSize"); it != l3Json.end())
    lineSize = (*it)["value"].get<int>();

  int id = 0;
  for (auto leaf : leafs) {
    for (int i = 0; i < amountPerLeaf; i++) {
      l3Caches[id] = new Cache(leaf, id, "L3", size, -1, lineSize);
      id++;
    }
  }

  // the existence of `readBandwidth` implies the existence of `writeBandwidth`
  if (auto it = l3Json.find("readBandwidth"); it != l3Json.end()) {
    double readBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );
    double writeBandwidth = GiBs_to_Bs( l3Json["writeBandwidth"]["value"].get<double>() );

    for (auto l3Cache : l3Caches) {
      for (auto core : cores) {
        auto dp = new DataPath(l3Cache, core, DataPathOrientation::Oriented,
                               DataPathType::Logical, -1, -1);
        dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
        dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
      }
    }
  }

  leafs = std::move(l3Caches);
}

static void ParseL2Cache(const json &l2Json, std::vector<Thread *> &cores,
                         std::vector<Component *> &leafs)
{
  int amount = l2Json.value("amount", 1);
  int amountPerLeaf = amount / leafs.size();
  std::vector<Component *> l2Caches (amount);

  long long size = l2Json["size"]["value"].get<long long>();

  int lineSize = -1;
  if (auto it = l2Json.find("lineSize"); it != l2Json.end()) {
    auto valueIt = it->find("value");
    lineSize = valueIt != it->end() ? valueIt->get<int>() : (*it)["size"].get<int>();
  }

  int fetchGranularity = -1;
  if (auto it = l2Json.find("fetchGranularity"); it != l2Json.end())
    fetchGranularity = (*it)["size"].get<int>();

  int segmentSize = -1;
  if (auto it = l2Json.find("segmentSize"); it != l2Json.end())
    segmentSize = (*it)["size"].get<int>();

  int id = 0;
  for (auto leaf : leafs) {
    for (int i = 0; i < amountPerLeaf; i++) {
      l2Caches[id] = new Cache(leaf, id, "L2", size, -1, lineSize);

      if (fetchGranularity > 0)
        l2Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new double(fetchGranularity) );
      if (segmentSize > 0)
        l2Caches[id]->attrib["segmentSize"] = reinterpret_cast<void *>( new double(segmentSize) );

      id++;
    }
  }

  if (auto it = l2Json.find("latency"); it != l2Json.end()) {
    double latency = (*it)["mean"].get<double>();
    double readBandwidth = GiBs_to_Bs( l2Json["readBandwidth"]["value"].get<double>() );
    double writeBandwidth = GiBs_to_Bs( l2Json["writeBandwidth"]["value"].get<double>() );
    double missPenalty = -1;
    if (auto missPenaltyIt = l2Json.find("missPenalty"); missPenaltyIt != l2Json.end())
      missPenalty = (*missPenaltyIt)["value"].get<double>();

    for (auto l2Cache : l2Caches) {
      for (auto core : cores) {
        auto dp = new DataPath(l2Cache, core, DataPathOrientation::Oriented,
                               DataPathType::Logical, -1, latency);
        dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
        dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
        if (missPenalty > 0)
          dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
      }
    }
  }

  leafs = std::move(l2Caches);
}

static void ParseScalarL1Cache(const json &scalarL1Json,
                               std::vector<Subdivision *> &mps,
                               std::vector<Thread *> &cores,
                               std::vector<Component *> &leafs)
{
  int uniqueAmount = scalarL1Json.value("uniqueAmount", 1);
  int amountPerLeaf = uniqueAmount / leafs.size();
  std::vector<Component *> scalarL1Caches (uniqueAmount);

  long long size = scalarL1Json["size"]["size"].get<long long>();

  long long lineSize = -1;
  if (auto it = scalarL1Json.find("lineSize"); it != scalarL1Json.end())
    lineSize = (*it)["size"].get<size_t>();

  size_t fetchGranularity = scalarL1Json["fetchGranularity"]["size"].get<size_t>();

  auto sharedBetweenJsonIt = scalarL1Json.find("sharedBetween");

  auto mpIt = mps.begin();
  int id = 0;
  for (auto leaf : leafs) {
    for (int i = 0; i < amountPerLeaf; i++) {
      scalarL1Caches[id] = new Cache(leaf, id, "Scalar L1", size, -1, lineSize);

      scalarL1Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>(
        new size_t (fetchGranularity)
      );

      if (sharedBetweenJsonIt != scalarL1Json.end() && sharedBetweenJsonIt->size() > 0) {
        for (const auto &elem : (*sharedBetweenJsonIt)[id]) {
          (*mpIt)->SetId(elem.get<int>());
          scalarL1Caches[id]->InsertChild(*mpIt);
          mpIt++;
        }
      }

      id++;
    }
  }

  double latency = scalarL1Json["latency"]["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = scalarL1Json.find("missPenalty"); it != scalarL1Json.end())
    missPenalty = (*it)["value"].get<double>();

  auto coreIt = cores.begin();
  int numCoresPerMP = cores.size() / mps.size();
  for (auto scalarL1Cache : scalarL1Caches) {
    int numCores = scalarL1Cache->GetChildren().size() * numCoresPerMP;
    for (int i = 0; i < numCores; i++) {
      auto dp = new DataPath(scalarL1Cache, *coreIt, DataPathOrientation::Oriented,
                             DataPathType::Logical, -1, latency);
      if (missPenalty > 0)
        dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );

      coreIt++;
    }
  }

  leafs = std::move(scalarL1Caches);
}

static void ParseGlobalMemory(const json &memory, Chip *gpu,
                              std::vector<Subdivision *> &mps,
                              std::vector<Thread *> &cores,
                              std::vector<Component *> &leafs)
{
  ParseMainMemory(memory["main"], gpu, cores, leafs);

  if (auto it = memory.find("l3"); it != memory.end())
    ParseL3Cache(*it, cores, leafs);

  ParseL2Cache(memory["l2"], cores, leafs);

  if (auto it = memory.find("scalarL1"); it != memory.end())
    ParseScalarL1Cache(*it, mps, cores, leafs);
}

int sys_sage::ParseMt4g(Component *parent, const std::string &path, int gpuId)
{
  if (!parent) {
    std::cerr << "ParseMt4g: parent is null\n";
    return 1;
  }

  std::ifstream file (path);
  if (file.fail()) {
    std::cerr << "ParseMt4g: could not open file '" << path << "'\n";
    return 1;
  }

  json data = json::parse(file);

  auto *gpu = new Chip(parent, gpuId, "GPU", ChipType::Gpu);

  ParseGeneral(data["general"], gpu);

  auto [numMPs, numCoresPerMP] = ParseCompute(data["compute"], gpu);

  std::vector<Subdivision *> mps (numMPs);
  std::vector<Thread *> cores (numMPs * numCoresPerMP);

  for (int i = 0; i < numMPs; i++) {
    mps[i] = new Subdivision(i, "Multiprocessor");
    mps[i]->SetSubdivisionType(sys_sage::SubdivisionType::GpuSM);
  }
  for (int i = 0; i < numMPs * numCoresPerMP; i++)
    cores[i] = new Thread(i, "GPU Core");

  std::vector<Component *> leafs;
  ParseGlobalMemory(data["memory"], gpu, mps, cores, leafs);

  return 0;
}

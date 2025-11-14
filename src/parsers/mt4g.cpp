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
                            std::vector<Component *> &cores,
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

static void ParseL3Cache(const json &l3Json, std::vector<Component *> &cores,
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
  for (auto leaf : leafs)
    for (int i = 0; i < amountPerLeaf; i++, id++)
      l3Caches[id] = new Cache(leaf, id, "L3", size, -1, lineSize);

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

static void ParseL2Cache(const json &l2Json, std::vector<Component *> &cores,
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
    for (int i = 0; i < amountPerLeaf; i++, id++) {
      l2Caches[id] = new Cache(leaf, id, "L2", size, -1, lineSize);

      if (fetchGranularity > 0)
        l2Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new double(fetchGranularity) );
      if (segmentSize > 0)
        l2Caches[id]->attrib["segmentSize"] = reinterpret_cast<void *>( new double(segmentSize) );
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

static bool ParseScalarL1Cache(const json &scalarL1Json,
                               std::vector<Component *> &mps,
                               std::vector<Component *> &cores,
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
  bool insertMPs = sharedBetweenJsonIt != scalarL1Json.end() && sharedBetweenJsonIt->size() > 0;

  auto mpIt = mps.begin();
  int id = 0;
  for (auto leaf : leafs) {
    for (int i = 0; i < amountPerLeaf; i++, id++) {
      scalarL1Caches[id] = new Cache(leaf, id, "Scalar L1", size, -1, lineSize);

      scalarL1Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>(
        new size_t (fetchGranularity)
      );

      if (insertMPs) {
        for (const auto &elem : (*sharedBetweenJsonIt)[id]) {
          (*mpIt)->SetId(elem.get<int>());
          scalarL1Caches[id]->InsertChild(*mpIt);
          mpIt++;
        }
      }
    }
  }

  double latency = scalarL1Json["latency"]["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = scalarL1Json.find("missPenalty"); it != scalarL1Json.end())
    missPenalty = (*it)["value"].get<double>();

  int defaultAmountMPsPerScalarL1Cache = mps.size() / uniqueAmount;
  int numCoresPerMP = cores.size() / mps.size();

  auto coreIt = cores.begin();
  for (auto scalarL1Cache : scalarL1Caches) {
    int numCores = insertMPs ? scalarL1Cache->GetChildren().size() : defaultAmountMPsPerScalarL1Cache;
    numCores *= numCoresPerMP;

    for (int i = 0; i < numCores; i++, coreIt++) {
      auto dp = new DataPath(scalarL1Cache, *coreIt, DataPathOrientation::Oriented,
                             DataPathType::Logical, -1, latency);
      if (missPenalty > 0)
        dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
    }
  }

  leafs = std::move(scalarL1Caches);

  return insertMPs;
}

static void ParseL1Cache(const json &l1Json, std::vector<Component *> &cores,
                         int numCoresPerMP, std::vector<Component *> &leafs)
{
  // this could be problematic if the leafs aren't the MPs
  int amountPerLeaf = l1Json.value("amountPerMultiprocessor", 1);
  std::vector<Component *> l1Caches ( amountPerLeaf * leafs.size() );

  long long size = -1;
  if (auto it = l1Json.find("size"); it != l1Json.end()) {
    size = (*it)["size"].get<long long>();
  }

  int lineSize = -1;
  if (auto it = l1Json.find("lineSize"); it != l1Json.end()) {
    lineSize = (*it)["size"].get<int>();
  }

  int fetchGranularity = -1;
  if (auto it = l1Json.find("fetchGranularity"); it != l1Json.end())
    fetchGranularity = (*it)["size"].get<int>();

  int id = 0;
  for (auto leaf : leafs) {
    for (int i = 0; i < amountPerLeaf; i++, id++) {
      l1Caches[id] = new Cache(leaf, id, "L1", size, -1, lineSize);

      if (fetchGranularity > 0)
        l1Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new double(fetchGranularity) );
    }
  }

  if (auto it = l1Json.find("latency"); it != l1Json.end()) {
    double latency = (*it)["mean"].get<double>();
    
    double missPenalty = -1;
    if (auto missPenaltyIt = l1Json.find("missPenalty"); missPenaltyIt != l1Json.end())
      missPenalty = (*missPenaltyIt)["value"].get<double>();

    //TODO: fix
    //auto coreIt = cores.begin();
    //for (auto l1Cache : l1Caches) {
    //  for (int i = 0; i < numCoresPerMP; i++, coreIt++) {
    //    auto dp = new DataPath(l1Cache, *coreIt, DataPathOrientation::Oriented,
    //                           DataPathType::Logical, -1, latency);

    //    if (missPenalty > 0)
    //      dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
    //  }
    //}

    int numMPs = leafs.size();
    for (int i = 0; i < numMPs; i++) {
      for (int j = 0; j < numCoresPerMP; j++) {
        for (int k = 0; k < amountPerLeaf; k++) {
          auto dp = new DataPath(l1Caches[k + i * amountPerLeaf], cores[j + i * numCoresPerMP], DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);

          if (missPenalty > 0)
            dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
        }
      }
    }
  }

  leafs = std::move(l1Caches);
}

static void ParseSharedMemory(const json &sharedJson,
                              std::vector<Component *> &cores, int numCoresPerMP,
                              std::vector<Component *> &leafs)
{
  long long memPerBlock = sharedJson["sharedMemPerBlock"]["value"].get<long long>();
  long long memPerMultiProcessor = sharedJson["sharedMemPerMultiProcessor"]["value"].get<long long>();
  long long reservedSharedMemPerBlock = sharedJson["reservedSharedMemPerBlock"]["value"].get<long long>();

  double latency = -1;
  if (auto it = sharedJson.find("latency"); it != sharedJson.end())
    latency = (*it)["mean"].get<double>();

  auto coreIt = cores.begin();
  int id = 0;
  for (auto leaf : leafs) {
    auto sharedMem = new Memory(leaf, id++, "Shared Memory");

    sharedMem->attrib["memPerBlock"] = reinterpret_cast<void *>( new long long (memPerBlock) );
    sharedMem->attrib["memPerMultiProcessor"] = reinterpret_cast<void *>( new long long (memPerMultiProcessor) );
    sharedMem->attrib["reservedSharedMemPerBlock"] = reinterpret_cast<void *>( new long long (reservedSharedMemPerBlock) );

    if (latency > 0)
      for (int i = 0; i < numCoresPerMP; i++, coreIt++)
        new DataPath(sharedMem, *coreIt, DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);
  }
}

static void ParseConstantCache(const json &constantJson,
                               std::vector<Component *> &cores, int numCoresPerMP,
                               std::vector<Component *> &leafs)
{
  long long totalConstMem = constantJson["totalConstMem"]["value"].get<long long>();

  if (constantJson.size() == 1) {
    int id = 0;
    for (auto leaf : leafs) {
      auto constantCache = new Cache(leaf, id++, "Constant");
      constantCache->attrib["totalConstMem"] = reinterpret_cast<void *>(
        new long long (totalConstMem)
      );
    }

    return;
  }

  const json &constantL1_5Json = constantJson["l1.5"];
  std::vector<Component *> constantL1_5Caches (leafs.size());

  long long constantl1_5Size = constantL1_5Json["size"]["size"].get<long long>();
  int constantl1_5fetchGranularity = constantL1_5Json["fetchGranularity"]["size"].get<int>();

  int constantL1_5LineSize = -1;
  if (auto it = constantL1_5Json.find("lineSize"); it != constantL1_5Json.end())
    constantL1_5LineSize = (*it)["size"].get<int>();

  int id = 0;
  for (auto leaf : leafs) {
    constantL1_5Caches[id] = new Cache(leaf, id, "Constant L1.5", constantl1_5Size,
                                       -1, constantL1_5LineSize);

    constantL1_5Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>(
      new int (constantl1_5fetchGranularity)
    );

    id++;
  }

  if (auto it = constantL1_5Json.find("latency"); it != constantL1_5Json.end()) {
    double latency = (*it)["mean"].get<double>();

    auto coreIt = cores.begin();
    for (auto constantL1_5Cache : constantL1_5Caches)
      for (int i = 0; i < numCoresPerMP; i++, coreIt++)
        new DataPath(constantL1_5Cache, *coreIt, DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);
  }

  const json &constantL1Json = constantJson["l1"];

  if (auto it = constantL1Json.find("sharedWith"); it != constantL1Json.end())
    return; // contained in either L1, Texture or ReadOnly

  int amountPerMP = constantL1Json.value("amountPerMultiprocessor", 1);
  std::vector<Component *> constantL1Caches ( leafs.size() * amountPerMP );

  long long constantl1Size = constantL1Json["size"]["size"].get<long long>();
  int constantl1fetchGranularity = constantL1Json["fetchGranularity"]["size"].get<int>();

  int constantL1LineSize = -1;
  if (auto it = constantL1Json.find("lineSize"); it != constantL1Json.end())
    constantL1LineSize = (*it)["size"].get<int>();

  id = 0;
  for (auto constantL1_5Cache : constantL1_5Caches) {
    for (int i = 0; i < amountPerMP; i++, id++) {
      constantL1Caches[id] = new Cache(constantL1_5Cache, id, "Constant L1", constantL1LineSize, -1, constantl1Size);
      constantL1Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>(
        new int (constantl1fetchGranularity)
      );
    }
  }

  double latency = constantL1Json["latency"]["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = constantL1Json.find("missPenalty"); it != constantL1Json.end())
    missPenalty = (*it)["value"].get<double>();

  int numMPs = leafs.size();
  for (int i = 0; i < numMPs; i++) {
    for (int j = 0; j < numCoresPerMP; j++) {
      for (int k = 0; k < amountPerMP; k++) {
        auto dp = new DataPath(constantL1Caches[k + i * amountPerMP], cores[j + i * numCoresPerMP], DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);

        if (missPenalty > 0)
          dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
      }
    }
  }
}

// `leafs` is assumed to be empty
static void ParseGlobalMemory(const json &memory, Chip *gpu,
                              std::vector<Component *> &mps,
                              std::vector<Component *> &cores,
                              std::vector<Component *> &leafs)
{
  ParseMainMemory(memory["main"], gpu, cores, leafs);

  if (auto it = memory.find("l3"); it != memory.end())
    ParseL3Cache(*it, cores, leafs);

  ParseL2Cache(memory["l2"], cores, leafs);

  if (auto it = memory.find("scalarL1"); it != memory.end() && ParseScalarL1Cache(*it, mps, cores, leafs))
    return;

  int amountPerLeaf = mps.size() / leafs.size();
  auto mpIt = mps.begin();
  for (auto leaf : leafs)
    for (int i = 0; i < amountPerLeaf; i++, mpIt++)
      leaf->InsertChild(*mpIt);
}

// `leafs` is assumed to contain the MPs
static void ParseLocalMemory(const json &memory, std::vector<Component *> &cores,
                             int numCoresPerMP, std::vector<Component *> &leafs)
{
  ParseSharedMemory(memory["shared"], cores, numCoresPerMP, leafs);

  ParseConstantCache(memory["constant"], cores, numCoresPerMP, leafs);

  ParseL1Cache(memory["l1"], cores, numCoresPerMP, leafs);
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

  std::vector<Component *> mps (numMPs);
  std::vector<Component *> cores (numMPs * numCoresPerMP);

  for (int i = 0; i < numMPs; i++) {
    mps[i] = new Subdivision(i, "Multiprocessor");
    static_cast<Subdivision *>(mps[i])->SetSubdivisionType(sys_sage::SubdivisionType::GpuSM);
  }
  for (int i = 0; i < numMPs * numCoresPerMP; i++)
    cores[i] = new Thread(i, "GPU Core");

  std::vector<Component *> leafs;
  ParseGlobalMemory(data["memory"], gpu, mps, cores, leafs);

  // at this point, the leafs are the lowest level of global memory/cache
  // and the MPs are inserted as the leafs' children

  leafs = std::move(mps);
  mps.clear(); // do not use mps anymore

  ParseLocalMemory(data["memory"], cores, numCoresPerMP, leafs);

  return 0;
}

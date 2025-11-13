#include "mt4g.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>
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
  auto *multiProcessorCount = new int( compute["multiProcessorCount"].get<int>() );
  gpu->attrib["multiProcessorCount"] = reinterpret_cast<void *>( multiProcessorCount );

  auto *numberOfCoresPerMultiProcessor = new int( compute["numberOfCoresPerMultiProcessor"].get<int>() );
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

static void ParseMainMemory(const json &mainJson, Chip *gpu, Memory **mainOut,
                            std::vector<Thread *> &cores)
{
  auto *main = new Memory(gpu, 0, "GPU Main Memory",
                          mainJson["totalGlobalMem"]["value"].get<long long>());
  *mainOut = main;

  main->attrib["clockRate"] = reinterpret_cast<void *>(
    new double ( kHz_to_Hz(mainJson["memoryClockRate"]["value"].get<double>()) )
  );
  main->attrib["busWidth"] = reinterpret_cast<void *>(
    new int ( mainJson["memoryBusWidth"]["value"].get<int>() )
  );

  double latency = -1;
  if (auto it = mainJson.find("latency"); it != mainJson.end())
    latency = (*it)["mean"].get<double>();

  double readBandwidth = -1;
  if (auto it = mainJson.find("readBandwidth"); it != mainJson.end())
    readBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );

  double writeBandwidth = -1;
  if (auto it = mainJson.find("writeBandwidth"); it != mainJson.end())
    writeBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );

  if (latency > 0 || readBandwidth > 0 || writeBandwidth > 0) {
    for (auto *core : cores) {
      auto *dp = new DataPath(main, core, DataPathOrientation::Oriented,
                              DataPathType::Logical, -1, latency);
      if (readBandwidth > 0)
        dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
      if (writeBandwidth > 0)
        dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
    }
  }
}

// assume `l3Caches` is empty
static void ParseL3Cache(const json &l3Json, Memory *main,
                         std::vector<Component *> &l3Caches,
                         std::vector<Thread *> &cores)
{
  int amount = l3Json.value("amount", 1);
  l3Caches.reserve(amount);

  long long size = l3Json["size"]["value"].get<long long>();

  int lineSize = -1;
  if (auto it = l3Json.find("lineSize"); it != l3Json.end())
    lineSize = (*it)["value"].get<int>();

  for (int i = 0; i < amount; i++)
    l3Caches.push_back(new Cache(main, i, "L3", size, -1, lineSize));

  double readBandwidth = -1;
  if (auto it = l3Json.find("readBandwidth"); it != l3Json.end())
    readBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );

  double writeBandwidth = -1;
  if (auto it = l3Json.find("writeBandwidth"); it != l3Json.end())
    writeBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );

  if (readBandwidth > 0 || writeBandwidth > 0) {
    for (auto *l3Cache : l3Caches) {
      for (auto *core : cores) {
        auto *dp = new DataPath(l3Cache, core, DataPathOrientation::Oriented,
                                DataPathType::Logical, -1, -1);
        if (readBandwidth > 0)
          dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
        if (writeBandwidth > 0)
          dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
      }
    }
  }
}

static void ParseL2Cache(const json &l2Json, std::vector<Component *> &parents, 
                         std::vector<Component *> &l2Caches,
                         std::vector<Thread *> &cores)
{
  int amount = l2Json.value("amount", 1);
  l2Caches.reserve(amount);
  int amountPerParent = amount / parents.size();

  long long size = l2Json["size"]["value"].get<long long>();

  int lineSize = -1;
  if (auto it = l2Json.find("lineSize"); it != l2Json.end()) {
    if (auto it2 = it->find("value"); it2 != it->end())
      lineSize = it2->get<int>();
    else
      lineSize = (*it)["size"].get<int>();
  }

  int fetchGranularity = -1;
  if (auto it = l2Json.find("fetchGranularity"); it != l2Json.end())
    fetchGranularity = (*it)["size"].get<int>();

  int segmentSize = -1;
  if (auto it = l2Json.find("segmentSize"); it != l2Json.end())
    segmentSize = (*it)["size"].get<int>();

  int id = 0;
  for (Component *parent : parents) {
    for (int i = 0; i < amountPerParent; i++) {
      auto *l2Cache = new Cache(parent, id++, "L2", size, -1, lineSize);

      if (fetchGranularity > 0)
        l2Cache->attrib["fetchGranularity"] = reinterpret_cast<void *>( new double(fetchGranularity) );
      if (segmentSize > 0)
        l2Cache->attrib["segmentSize"] = reinterpret_cast<void *>( new double(segmentSize) );

      l2Caches.push_back(l2Cache);
    }
  }

  double latency = -1;
  if (auto it = l2Json.find("latency"); it != l2Json.end())
    latency = (*it)["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = l2Json.find("missPenalty"); it != l2Json.end())
    missPenalty = (*it)["value"].get<double>();

  double readBandwidth = -1;
  if (auto it = l2Json.find("readBandwidth"); it != l2Json.end())
    readBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );

  double writeBandwidth = -1;
  if (auto it = l2Json.find("writeBandwidth"); it != l2Json.end())
    writeBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );

  if (latency > 0 || missPenalty > 0 || readBandwidth > 0 || writeBandwidth > 0) {
    for (auto *l2Cache : l2Caches) {
      for (auto *core : cores) {
        auto *dp = new DataPath(l2Cache, core, DataPathOrientation::Oriented,
                                DataPathType::Logical, -1, latency);
        if (missPenalty > 0)
          dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
        if (readBandwidth > 0)
          dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
        if (writeBandwidth > 0)
          dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
      }
    }
  }
}

static void ParseScalarL1Cache(const json &scalarL1Json,
                               std::vector<Component *> &parents,
                               std::vector<Component *> &scalarL1Caches,
                               std::vector<Thread *> &cores)
{
  size_t fetchGranularity = scalarL1Json["fetchGranularity"]["size"].get<size_t>();
  size_t size = scalarL1Json["size"]["size"].get<size_t>();

  std::optional<size_t> lineSize = std::nullopt;
  if (auto it = scalarL1Json.find("lineSize"); it != scalarL1Json.end())
    lineSize = (*it)["size"].get<size_t>();

  double missPenalty = -1;
  if (auto it = scalarL1Json.find("missPenalty"); it != scalarL1Json.end())
    missPenalty = (*it)["value"].get<double>();

  double frequency = scalarL1Json["frequency"]["mean"].get<double>();
}

static void ParseGlobalMemory(const json &memory, Chip *gpu,
                              std::vector<Thread *> &cores)
{
  Memory *main;
  ParseMainMemory(memory["main"], gpu, &main, cores);

  std::vector<Component *> components;

  if (auto it = memory.find("l3"); it != memory.end())
    ParseL3Cache(*it, main, components, cores);
  else
    components.push_back(main);

  std::vector<Component *> l2Caches;
  ParseL2Cache(memory["l2"], components, l2Caches, cores);

  if (auto it = memory.find("scalarL1"); it != memory.end()) {
    std::vector<Component *> scalarL1Caches;
    ParseScalarL1Cache(*it, l2Caches, scalarL1Caches, cores);
    components = std::move(scalarL1Caches);
  } else {
    components = std::move(l2Caches);
    l2Caches.clear();
  }
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
  std::vector<Thread *> cores (numMPs * numCoresPerMP);

  int id = 0;
  for (int i = 0; i < numMPs; i++) {
    auto *mp = new Subdivision(i, "Multiprocessor");
    mp->SetSubdivisionType(sys_sage::SubdivisionType::GpuSM);
    for (int j = 0; j < numCoresPerMP; j++) {
      cores[id] = new Thread(id, "GPU Core");
      id++;
    }
  }

  ParseGlobalMemory(data["memory"], gpu, cores);

  return 0;
}

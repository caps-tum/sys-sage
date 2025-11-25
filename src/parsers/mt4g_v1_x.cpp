#include "mt4g.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace sys_sage;
using json = nlohmann::json;

static inline long long kHz_to_Hz(long long kHz)
{
  return kHz * 1000;
}

static inline double GiBs_to_Bs(double GiBs)
{
  return GiBs * (1 << 30);
}

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
    // the clockRate field is actually of type `int`, but due to the conversion
    // from [kHz] to [Hz], it might be better to use `long long` to avoid
    // overflows
    new long long( kHz_to_Hz(general["clockRate"]["value"].get<int>()) )
  );
}

static std::tuple<int, uint32_t> ParseCompute(const json &compute, Chip *gpu)
{
  auto multiProcessorCount = new int( compute["multiProcessorCount"].get<int>() );
  gpu->attrib["multiProcessorCount"] = reinterpret_cast<void *>( multiProcessorCount );

  auto numberOfCoresPerMultiProcessor = new uint32_t( compute["numberOfCoresPerMultiProcessor"].get<uint32_t>() );
  gpu->attrib["numberOfCoresPerMultiProcessor"] = reinterpret_cast<void *>( numberOfCoresPerMultiProcessor );

  gpu->attrib["maxThreadsPerBlock"] = reinterpret_cast<void *>(
    new int( compute["maxThreadsPerBlock"].get<int>() )
  );

  // TODO: maybe parse register info?

  gpu->attrib["warpSize"] = reinterpret_cast<void *>(
    new int( compute["warpSize"].get<int>() )
  );
  gpu->attrib["maxThreadsPerMultiProcessor"] = reinterpret_cast<void *>(
    new int( compute["maxThreadsPerMultiProcessor"].get<int>() )
  );
  gpu->attrib["maxBlocksPerMultiProcessor"] = reinterpret_cast<void *>(
    new int( compute["maxBlocksPerMultiProcessor"].get<int>() )
  );

  if (auto it = compute.find("numXDCDs"); it != compute.end())
    gpu->attrib["numXDCDs"] = reinterpret_cast<void *>( new uint32_t(it->get<uint32_t>()) );

  if (auto it = compute.find("computeUnitsPerDie"); it != compute.end())
    gpu->attrib["computeUnitsPerDie"] = reinterpret_cast<void *>( new uint32_t(it->get<uint32_t>()) );

  if (auto it = compute.find("numSIMDsPerCu"); it != compute.end())
    gpu->attrib["numSIMDsPerCu"] = reinterpret_cast<void *>( new uint32_t(it->get<uint32_t>()) );


  return { *multiProcessorCount, *numberOfCoresPerMultiProcessor };
}

// assumes `leafs` only contains the GPU
static void ParseMainMemory(const json &main, std::vector<Component *> &cores,
                            std::vector<Component *> &leafs)
{
  size_t size = main["totalGlobalMem"]["value"].get<size_t>();

  auto mainMem = new Memory(leafs[0], 0, "GPU Main Memory", size);

  mainMem->attrib["clockRate"] = reinterpret_cast<void *>(
    new long long ( kHz_to_Hz(main["memoryClockRate"]["value"].get<int>()) )
  );
  mainMem->attrib["busWidth"] = reinterpret_cast<void *>(
    new int ( main["memoryBusWidth"]["value"].get<int>() )
  );

  // the existence of `latency` implies the existence of `readBandwidth` and `writeBandwidth`
  if (auto it = main.find("latency"); it != main.end()) {
    double latency = (*it)["mean"].get<double>();
    double readBandwidth = GiBs_to_Bs( main["readBandwidth"]["value"].get<double>() );
    double writeBandwidth = GiBs_to_Bs( main["writeBandwidth"]["value"].get<double>() );

    for (auto core : cores) {
      // bidirectional, because we have read & write bandwidth
      auto dp = new DataPath(mainMem, core, DataPathOrientation::Bidirectional,
                             DataPathType::Logical, -1, latency);
      dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
      dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
    }
  }

  // update `leafs` to only contain the main memory
  leafs[0] = mainMem;
}

static void ParseL3Caches(const json &l3, std::vector<Component *> &cores,
                          std::vector<Component *> &leafs)
{
  long long size = -1;
  if (auto it = l3.find("size"); it != l3.end())
    size = (*it)["value"].get<size_t>();

  size_t amount = l3.value("amount", 1);
  std::vector<Component *> l3Caches (amount);

  size_t amountPerLeaf = amount / leafs.size();

  int lineSize = -1;
  if (auto it = l3.find("lineSize"); it != l3.end())
    lineSize = (*it)["value"].get<size_t>();

  size_t id = 0;

  for (auto leaf : leafs)
    for (size_t i = 0; i < amountPerLeaf; i++, id++)
      l3Caches[id] = new Cache(leaf, id, "L3", size, -1, lineSize);

  // the existence of `readBandwidth` implies the existence of `writeBandwidth`
  if (auto it = l3.find("readBandwidth"); it != l3.end()) {
    double readBandwidth = GiBs_to_Bs( (*it)["value"].get<double>() );
    double writeBandwidth = GiBs_to_Bs( l3["writeBandwidth"]["value"].get<double>() );

    for (auto l3Cache : l3Caches) {
      for (auto core : cores) {
        auto dp = new DataPath(l3Cache, core, DataPathOrientation::Bidirectional,
                               DataPathType::Logical, -1, -1);
        dp->attrib["readBandwidth"] = reinterpret_cast<void *>( new double (readBandwidth) );
        dp->attrib["writeBandwidth"] = reinterpret_cast<void *>( new double (writeBandwidth) );
      }
    }
  }

  leafs = std::move(l3Caches);
}

static void ParseL2Caches(const json &l2, std::vector<Component *> &cores,
                          std::vector<Component *> &leafs)
{
  long long size = l2["size"]["value"].get<int>();

  size_t amount = l2.value("amount", 1);
  std::vector<Component *> l2Caches (amount);

  size_t amountPerLeaf = amount / leafs.size();

  int lineSize = -1;
  if (auto it = l2.find("lineSize"); it != l2.end()) {
    auto valueIt = it->find("value");
    lineSize = valueIt != it->end() ? valueIt->get<size_t>() : (*it)["size"].get<size_t>();
  }

  size_t fetchGranularity = 0;
  if (auto it = l2.find("fetchGranularity"); it != l2.end())
    fetchGranularity = (*it)["size"].get<size_t>();

  size_t segmentSize = 0;
  if (auto it = l2.find("segmentSize"); it != l2.end())
    segmentSize = (*it)["size"].get<size_t>();

  size_t id = 0;

  for (auto leaf : leafs) {
    for (size_t i = 0; i < amountPerLeaf; i++, id++) {
      l2Caches[id] = new Cache(leaf, id, "L2", size, -1, lineSize);

      if (fetchGranularity > 0)
        l2Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new size_t(fetchGranularity) );
      if (segmentSize > 0)
        l2Caches[id]->attrib["segmentSize"] = reinterpret_cast<void *>( new size_t(segmentSize) );
    }
  }

  if (auto it = l2.find("latency"); it != l2.end()) {
    double latency = (*it)["mean"].get<double>();
    double readBandwidth = GiBs_to_Bs( l2["readBandwidth"]["value"].get<double>() );
    double writeBandwidth = GiBs_to_Bs( l2["writeBandwidth"]["value"].get<double>() );
    double missPenalty = -1;
    if (auto missPenaltyIt = l2.find("missPenalty"); missPenaltyIt != l2.end())
      missPenalty = (*missPenaltyIt)["value"].get<double>();

    for (auto l2Cache : l2Caches) {
      for (auto core : cores) {
        auto dp = new DataPath(l2Cache, core, DataPathOrientation::Bidirectional,
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

static bool ParseScalarL1Caches(const json &scalarL1,
                                std::vector<Component *> &mps,
                                std::vector<Component *> &cores,
                                std::vector<Component *> &leafs)
{
  size_t fetchGranularity = scalarL1["fetchGranularity"]["size"].get<size_t>();

  size_t size = scalarL1["size"]["size"].get<size_t>();

  int lineSize = -1;
  if (auto it = scalarL1.find("lineSize"); it != scalarL1.end())
    lineSize = (*it)["size"].get<size_t>();

  auto sharedBetweenIt = scalarL1.find("sharedBetween");
  bool insertMPs = sharedBetweenIt != scalarL1.end() && sharedBetweenIt->size() > 0;

  size_t uniqueAmount = scalarL1.value("uniqueAmount", 1);
  std::vector<Component *> scalarL1Caches (uniqueAmount);

  size_t amountPerLeaf = uniqueAmount / leafs.size();

  auto mpIt = mps.begin();
  size_t id = 0;

  for (auto leaf : leafs) {
    for (size_t i = 0; i < amountPerLeaf; i++, id++) {
      scalarL1Caches[id] = new Cache(leaf, id, "Scalar L1", size, -1, lineSize);

      scalarL1Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>(
        new size_t (fetchGranularity)
      );

      if (insertMPs) {
        for (const auto &elem : (*sharedBetweenIt)[id]) {
          (*mpIt)->SetId(elem.get<int>());
          scalarL1Caches[id]->InsertChild(*mpIt);
          mpIt++;
        }
      }
    }
  }

  double latency = scalarL1["latency"]["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = scalarL1.find("missPenalty"); it != scalarL1.end())
    missPenalty = (*it)["value"].get<double>();

  size_t defaultAmountMPsPerScalarL1Cache = mps.size() / uniqueAmount;
  size_t numCoresPerMP = cores.size() / mps.size();

  auto coreIt = cores.begin();

  for (auto scalarL1Cache : scalarL1Caches) {
    size_t numMPs = insertMPs ? scalarL1Cache->GetChildren().size() : defaultAmountMPsPerScalarL1Cache;
    size_t numCores = numMPs * numCoresPerMP;

    for (size_t i = 0; i < numCores; i++, coreIt++) {
      auto dp = new DataPath(scalarL1Cache, *coreIt, DataPathOrientation::Oriented,
                             DataPathType::Logical, -1, latency);
      if (missPenalty > 0)
        dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
    }
  }

  leafs = std::move(scalarL1Caches);

  return insertMPs;
}

static void ParseConstantCaches(const json &constant,
                               std::vector<Component *> &mps,
                               std::vector<Component *> &cores)
{
  size_t numCoresPerMP = cores.size() / mps.size();

  if (constant.size() == 1) {
    std::vector<Component *> constantCaches (mps.size());

    size_t totalConstMem = constant["totalConstMem"]["value"].get<size_t>();

    size_t id = 0;

    for (auto mp : mps) {
      constantCaches[id] = new Cache(mp, id, "Constant", totalConstMem);
      id++;
    }

    auto coreIt = cores.begin();

    for (auto constantCache : constantCaches)
      for (size_t i = 0; i < numCoresPerMP; i++, coreIt++)
        new DataPath(constantCache, *coreIt, DataPathOrientation::Oriented, DataPathType::Logical);

    return;
  }

  const json &cL1_5 = constant["l1.5"];

  size_t cL1_5FetchGranularity = cL1_5["fetchGranularity"]["size"].get<size_t>();

  size_t cL1_5Size = cL1_5["size"]["size"].get<size_t>();

  int cL1_5LineSize = -1;
  if (auto it = cL1_5.find("lineSize"); it != cL1_5.end())
    cL1_5LineSize = (*it)["size"].get<size_t>();

  // create one constant cache per MP
  std::vector<Component *> cL1_5Caches (mps.size());

  size_t cL1_5Id = 0;

  for (auto mp : mps) {
    cL1_5Caches[cL1_5Id] = new Cache(mp, cL1_5Id, "Constant L1.5", cL1_5Size, -1, cL1_5LineSize);

    cL1_5Caches[cL1_5Id]->attrib["fetchGranularity"] = reinterpret_cast<void *>(
      new size_t (cL1_5FetchGranularity)
    );

    cL1_5Id++;
  }

  if (auto it = cL1_5.find("latency"); it != cL1_5.end()) {
    double cL1_5Latency = (*it)["mean"].get<double>();

    auto coreIt = cores.begin();

    for (auto cL1_5Cache : cL1_5Caches)
      for (size_t i = 0; i < numCoresPerMP; i++, coreIt++)
        new DataPath(cL1_5Cache, *coreIt, DataPathOrientation::Oriented, DataPathType::Logical, -1, cL1_5Latency);
  }

  const json &cL1 = constant["l1"];

  if (auto it = cL1.find("sharedWith"); it != cL1.end())
    return; // contained in either L1, Texture or ReadOnly

  size_t cL1FetchGranularity = cL1["fetchGranularity"]["size"].get<size_t>();

  size_t cL1Size = cL1["size"]["size"].get<size_t>();

  int cL1LineSize = -1;
  if (auto it = cL1.find("lineSize"); it != cL1.end())
    cL1LineSize = (*it)["size"].get<size_t>();

  uint32_t amountPerMP = cL1.value("amountPerMultiprocessor", 1);
  std::vector<Component *> cL1Caches ( mps.size() * amountPerMP );

  size_t cL1Id = 0;

  for (auto cL1_5Cache : cL1_5Caches) {
    for (uint32_t i = 0; i < amountPerMP; i++, cL1Id++) {
      cL1Caches[cL1Id] = new Cache(cL1_5Cache, cL1Id, "Constant L1", cL1LineSize, -1, cL1Size);
      cL1Caches[cL1Id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new size_t (cL1FetchGranularity) );
    }
  }

  double cL1Latency = cL1["latency"]["mean"].get<double>();

  double cL1MissPenalty = -1;
  if (auto it = cL1.find("missPenalty"); it != cL1.end())
    cL1MissPenalty = (*it)["value"].get<double>();

  for (size_t i = 0; i < mps.size(); i++) {
    for (size_t j = 0; j < numCoresPerMP; j++) {
      for (uint32_t k = 0; k < amountPerMP; k++) {
        auto dp = new DataPath(cL1Caches[k + i * amountPerMP], cores[j + i * numCoresPerMP], DataPathOrientation::Oriented, DataPathType::Logical, -1, cL1Latency);

        if (cL1MissPenalty > 0)
          dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (cL1MissPenalty) );
      }
    }
  }
}

static void ParseSharedMemory(const json &shared, std::vector<Component *> &mps,
                              std::vector<Component *> &cores)
{
  size_t numCoresPerMP = cores.size() / mps.size();

  size_t memPerBlock = shared["sharedMemPerBlock"]["value"].get<size_t>();

  size_t memPerMultiProcessor = shared["sharedMemPerMultiProcessor"]["value"].get<size_t>();

  double latency = -1;
  if (auto it = shared.find("latency"); it != shared.end())
    latency = (*it)["mean"].get<double>();

  auto coreIt = cores.begin();

  size_t id = 0;

  for (auto mp : mps) {
    auto sharedMem = new Memory(mp, id++, "Shared Memory", memPerMultiProcessor);

    sharedMem->attrib["memPerBlock"] = reinterpret_cast<void *>( new long long (memPerBlock) );

    if (latency > 0)
      for (size_t i = 0; i < numCoresPerMP; i++, coreIt++)
        new DataPath(sharedMem, *coreIt, DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);
  }
}

static std::tuple<bool, bool>
ParseL1Caches(const json &l1, std::vector<Component *> &mps,
             std::vector<Component *> &cores)
{
  size_t numCoresPerMP = cores.size() / mps.size();

  size_t fetchGranularity = 0;
  if (auto it = l1.find("fetchGranularity"); it != l1.end())
    fetchGranularity = (*it)["size"].get<size_t>();

  long long size = -1;
  if (auto it = l1.find("size"); it != l1.end()) {
    size = (*it)["size"].get<size_t>();
  }

  int lineSize = -1;
  if (auto it = l1.find("lineSize"); it != l1.end()) {
    lineSize = (*it)["size"].get<size_t>();
  }

  uint32_t amountPerMP = l1.value("amountPerMultiprocessor", 1);
  std::vector<Component *> l1Caches ( amountPerMP * mps.size() );

  bool sharedWithTexture = false;
  bool sharedWithReadOnly = false;
  std::string name ( "L1" );

  if (auto it = l1.find("sharedWith"); it != l1.end()) {
    for (const auto &elem : *it) {
      const std::string elemName = elem.get<std::string>();

      if (elemName == "Texture")
        sharedWithTexture = true;
      else if (elemName == "Read Only")
        sharedWithReadOnly = true;

      name += "+" + elemName;
    }
  }

  size_t id = 0;

  for (auto mp : mps) {
    for (uint32_t i = 0; i < amountPerMP; i++, id++) {
      l1Caches[id] = new Cache(mp, id, name, size, -1, lineSize);

      if (fetchGranularity > 0)
        l1Caches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new double(fetchGranularity) );
    }
  }

  size_t amountCoresPerL1Cache = numCoresPerMP / amountPerMP;
  auto coreIt = cores.begin();

  for (auto l1Cache : l1Caches) {
    for (size_t i = 0; i < amountCoresPerL1Cache; i++, coreIt++)
      l1Cache->InsertChild(*coreIt);
  }

  if (auto it = l1.find("latency"); it != l1.end()) {
    double latency = (*it)["mean"].get<double>();
    
    double missPenalty = -1;
    if (auto missPenaltyIt = l1.find("missPenalty"); missPenaltyIt != l1.end())
      missPenalty = (*missPenaltyIt)["value"].get<double>();

    for (size_t i = 0; i < mps.size(); i++) {
      for (size_t j = 0; j < numCoresPerMP; j++) {
        for (uint32_t k = 0; k < amountPerMP; k++) {
          auto dp = new DataPath(l1Caches[k + i * amountPerMP], cores[j + i * numCoresPerMP], DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);

          if (missPenalty > 0)
            dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
        }
      }
    }
  }

  return { sharedWithTexture, sharedWithReadOnly };
}

static bool ParseTextureCaches(const json &texture, std::vector<Component *> &mps,
                               std::vector<Component *> &cores)
{
  size_t numCoresPerMP = cores.size() / mps.size();

  size_t fetchGranularity = texture["fetchGranularity"]["size"].get<size_t>();

  size_t size = texture["size"]["size"].get<size_t>();

  int lineSize = -1;
  if (auto it = texture.find("lineSize"); it != texture.end())
    lineSize = (*it)["size"].get<size_t>();

  uint32_t amountPerMP = texture.value("amountPerMultiprocessor", 1);
  std::vector<Component *> textureCaches ( amountPerMP * mps.size() );

  bool sharedWithReadOnly = false;
  std::string name ( "Texture" );

  if (auto it = texture.find("sharedWith"); it != texture.end()) {
    for (const auto &elem : *it) {
      const std::string elemName = elem.get<std::string>();

      if (elemName == "Read Only")
        sharedWithReadOnly = true;

      name += "+" + elemName;
    }
  }

  size_t id = 0;

  for (auto mp : mps) {
    for (uint32_t i = 0; i < amountPerMP; i++, id++) {
      textureCaches[id] = new Cache(mp, id, name, size, -1, lineSize);
      textureCaches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new int(fetchGranularity) );
    }
  }

  double latency = texture["latency"]["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = texture.find("missPenalty"); it != texture.end())
    missPenalty = (*it)["value"].get<double>();

  for (size_t i = 0; i < mps.size(); i++) {
    for (size_t j = 0; j < numCoresPerMP; j++) {
      for (uint32_t k = 0; k < amountPerMP; k++) {
        auto dp = new DataPath(textureCaches[k + i * amountPerMP], cores[j + i * numCoresPerMP], DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);

        if (missPenalty > 0)
          dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
      }
    }
  }

  return sharedWithReadOnly;
}

static void ParseReadOnlyCaches(const json &readOnly, std::vector<Component *> &mps,
                                std::vector<Component *> &cores)
{
  size_t numCoresPerMP = cores.size() / mps.size();

  size_t fetchGranularity = readOnly["fetchGranularity"]["size"].get<size_t>();

  size_t size = readOnly["size"]["size"].get<size_t>();

  int lineSize = -1;
  if (auto it = readOnly.find("lineSize"); it != readOnly.end())
    lineSize = (*it)["size"].get<size_t>();

  uint32_t amountPerMP = readOnly.value("amountPerMultiprocessor", 1);
  std::vector<Component *> readOnlyCaches ( amountPerMP * mps.size() );

  // no need to inspect the "sharedWith" entry, since it would be empty anyways
  // in this case

  size_t id = 0;

  for (auto mp : mps) {
    for (uint32_t i = 0; i < amountPerMP; i++, id++) {
      readOnlyCaches[id] = new Cache(mp, id, "Read Only", size, -1, lineSize);
      readOnlyCaches[id]->attrib["fetchGranularity"] = reinterpret_cast<void *>( new int(fetchGranularity) );
    }
  }

  double latency = readOnly["latency"]["mean"].get<double>();

  double missPenalty = -1;
  if (auto it = readOnly.find("missPenalty"); it != readOnly.end())
    missPenalty = (*it)["value"].get<double>();

  for (size_t i = 0; i < mps.size(); i++) {
    for (size_t j = 0; j < numCoresPerMP; j++) {
      for (uint32_t k = 0; k < amountPerMP; k++) {
        auto dp = new DataPath(readOnlyCaches[k + i * amountPerMP], cores[j + i * numCoresPerMP], DataPathOrientation::Oriented, DataPathType::Logical, -1, latency);

        if (missPenalty > 0)
          dp->attrib["missPenalty"] = reinterpret_cast<void *>( new double (missPenalty) );
      }
    }
  }
}

static void ParseGlobalMemory(const json &memory, Chip *gpu,
                              std::vector<Component *> &mps,
                              std::vector<Component *> &cores)
{
  std::vector<Component *> leafs { gpu };

  ParseMainMemory(memory["main"], cores, leafs);

  if (auto it = memory.find("l3"); it != memory.end())
    ParseL3Caches(*it, cores, leafs);

  ParseL2Caches(memory["l2"], cores, leafs);

  if (auto it = memory.find("scalarL1"); it != memory.end() && ParseScalarL1Caches(*it, mps, cores, leafs))
    return;

  size_t amountPerLeaf = mps.size() / leafs.size();
  auto mpIt = mps.begin();

  for (auto leaf : leafs)
    for (size_t i = 0; i < amountPerLeaf; i++, mpIt++)
      leaf->InsertChild(*mpIt);

  // at the end of the function, the leafs are the lowest level of global
  // memory/cache and the MPs are inserted as the leafs' children
}

static void ParseLocalMemory(const json &memory, std::vector<Component *> &mps,
                             std::vector<Component *> &cores)
{
  ParseConstantCaches(memory["constant"], mps, cores);

  ParseSharedMemory(memory["shared"], mps, cores);

  auto [l1SharedWithTexture, l1SharedWithReadOnly] = ParseL1Caches(memory["l1"], mps, cores);

  // at this point, the cores are inserted as the L1 caches' children

  bool textureSharedWithReadOnly = false;

  if (auto it = memory.find("texture"); it != memory.end() && !l1SharedWithTexture)
    textureSharedWithReadOnly = ParseTextureCaches(*it, mps, cores);

  if (auto it = memory.find("readOnly"); it != memory.end() && !l1SharedWithReadOnly && !textureSharedWithReadOnly)
    ParseReadOnlyCaches(*it, mps, cores);
}

int sys_sage::ParseMt4g_v1_x(Component *parent, const std::string &path, int gpuId)
{
  if (!parent) {
    std::cerr << "ParseMt4g: parent is nullptr\n";
    return 1;
  }

  auto *gpu = new Chip(parent, gpuId, "GPU", ChipType::Gpu);
  return ParseMt4g_v1_x(gpu, path);
}

int sys_sage::ParseMt4g_v1_x(Chip *gpu, const std::string &path)
{
  if (!gpu) {
    std::cerr << "ParseMt4g: gpu is nullptr\n";
    return 1;
  }

  std::ifstream file (path);
  if (file.fail()) {
    std::cerr << "ParseMt4g: could not open file '" << path << "'\n";
    return 1;
  }

  json data = json::parse(file);

  ParseGeneral(data["general"], gpu);

  auto [numMPs, numCoresPerMP] = ParseCompute(data["compute"], gpu);
  size_t numCores = numMPs * numCoresPerMP;

  std::vector<Component *> mps (numMPs);
  std::vector<Component *> cores (numCores);

  for (int i = 0; i < numMPs; i++) {
    mps[i] = new Subdivision(i, "Multiprocessor");
    static_cast<Subdivision *>(mps[i])->SetSubdivisionType(sys_sage::SubdivisionType::GpuSM);
  }
  for (size_t i = 0; i < numCores; i++)
    cores[i] = new Thread(i, "GPU Core");

  ParseGlobalMemory(data["memory"], gpu, mps, cores);

  ParseLocalMemory(data["memory"], mps, cores);

  return 0;
}

#include "mt4g.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <utility>

using namespace sys_sage;
using json = nlohmann::json;

static void ParseCompute(Chip *gpu, const json &compute)
{
  int *maxThreadsPerMultiProcessor = nullptr;
  int *multiProcessorCount = nullptr;

  for (auto &[key, value] : compute.items()) {
    if (key == "maxBlocksPerMultiProcessor"
        || key == "maxThreadsPerBlock"
        || key == "numberOfCoresPerMultiProcessor"
        || key == "warpSize") {
      int *i = new int( value.get<int>() );
      gpu->attrib[key] = reinterpret_cast<void *>( i );

    } else if (key == "maxThreadsPerMultiProcessor") {
      maxThreadsPerMultiProcessor = new int( value.get<int>() );
      gpu->attrib[key] = reinterpret_cast<void *>( maxThreadsPerMultiProcessor );

    } else if (key == "multiProcessorCount") {
      multiProcessorCount = new int( value.get<int>() );
      gpu->attrib[key] = reinterpret_cast<void *>( multiProcessorCount );
    }
  }
}

static void ParseGeneral(Chip *gpu, const json &general)
{
  for (auto &[key, value] : general.items()) {
    if (key == "clockRate") {
      double *rate = new double(value.at("value").get<double>());
      gpu->attrib[key] = reinterpret_cast<void *>( rate );
    } else if (key == "computeCapability") {
      std::pair<int, int> *majorMinor = new std::pair<int, int> {
                                              value.at("major").get<int>(),
                                              value.at("minor").get<int>()
                                            };
      gpu->attrib[key] = reinterpret_cast<void *>( majorMinor );
    } else if (key == "name") {
      gpu->SetModel(value.get<std::string>());
    } else if (key == "vendor") {
      gpu->SetVendor(value.get<std::string>());
    }
  }
}

static void ParseMemory(Chip *gpu, const json &memory)
{}

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

  Chip *gpu = new Chip(parent, gpuId, "GPU", ChipType::Gpu);

  ParseCompute(gpu, data.at("compute"));
  ParseGeneral(gpu, data.at("general"));
  ParseMemory(gpu, data.at("memory"));

  return 0;
}

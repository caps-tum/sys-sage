#include "mt4g.hpp"

int sys_sage::ParseMt4g(Component *parent, const std::string &path, int gpuId)
{
  return ParseMt4g_v1_x(parent, path, gpuId);
}

int sys_sage::ParseMt4g(Chip *gpu, const std::string &path)
{
  return ParseMt4g_v1_x(gpu, path);
}

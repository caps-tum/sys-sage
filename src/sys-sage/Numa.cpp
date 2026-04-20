#include "Numa.hpp"


sys_sage::Numa::Numa(int _id, long long _size):Subdivision(_id, "Numa", sys_sage::ComponentType::Numa), size(_size){}
sys_sage::Numa::Numa(Component * parent, int _id, long long _size):Subdivision(parent, _id, "Numa", sys_sage::ComponentType::Numa), size(_size) { }

long long sys_sage::Numa::GetSize() const{return size;}
void sys_sage::Numa::SetSize(long long _size) { size = _size;}

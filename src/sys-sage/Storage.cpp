#include "Storage.hpp"


sys_sage::Storage::Storage(long long _size):Component(0, "Storage", sys_sage::ComponentType::Storage), size(_size){}
sys_sage::Storage::Storage(Component * parent, long long _size):Component(parent, 0, "Storage", sys_sage::ComponentType::Storage), size(_size){}

void sys_sage::Storage::SetSize(long long _size){size = _size;} 
long long sys_sage::Storage::GetSize() const{return size;}
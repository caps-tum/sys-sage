#include "Memory.hpp"


sys_sage::Memory::Memory(long long _size, bool _is_volatile):Component(0, "Memory", sys_sage::ComponentType::Memory), size(_size), is_volatile(_is_volatile){}
sys_sage::Memory::Memory(Component * parent, int _id, std::string _name, long long _size, bool _is_volatile):Component(parent, _id, _name, sys_sage::ComponentType::Memory), size(_size), is_volatile(_is_volatile){}


long long sys_sage::Memory::GetSize() const {return size;}
void sys_sage::Memory::SetSize(long long _size) {size = _size;}
bool sys_sage::Memory::GetIsVolatile() const {return is_volatile;}
void sys_sage::Memory::SetIsVolatile(bool _is_volatile) {is_volatile = _is_volatile;}

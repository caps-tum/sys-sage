#include "Core.hpp"



sys_sage::Core::Core(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Core){}
sys_sage::Core::Core(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Core){}

#include "Thread.hpp"

sys_sage::Thread::Thread(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Thread){}
sys_sage::Thread::Thread(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Thread){}

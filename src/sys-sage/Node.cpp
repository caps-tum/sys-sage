#include "Node.hpp"


sys_sage::Node::Node(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Node){}
sys_sage::Node::Node(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Node){}

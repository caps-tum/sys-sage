#include "Subdivision.hpp"


sys_sage::Subdivision::Subdivision(int _id, std::string _name, sys_sage::ComponentType::type _componentType): Component(_id, _name, _componentType) { }
sys_sage::Subdivision::Subdivision(Component * parent, int _id, std::string _name, sys_sage::ComponentType::type _componentType): Component(parent, _id, _name, _componentType) { }
sys_sage::Subdivision::Subdivision(int _id, std::string _name): Component(_id, _name, sys_sage::ComponentType::Subdivision) { }
sys_sage::Subdivision::Subdivision(Component * parent, int _id, std::string _name): Component(parent, _id, _name, sys_sage::ComponentType::Subdivision) { }


//SVTODO should Subdivisiontype be settable?
void sys_sage::Subdivision::SetSubdivisionType(sys_sage::SubdivisionType::type subdivisionType){type = subdivisionType;}
sys_sage::SubdivisionType::type sys_sage::Subdivision::GetSubdivisionType() const {return type;}

#include <sys-sage/Subdivision.hpp>

sys_sage::Subdivision::Subdivision(int _id, const std::string &_name, sys_sage::ComponentType::type _componentType): Component(_id, _name, _componentType) { }
sys_sage::Subdivision::Subdivision(Component * parent, int _id, const std::string &_name, sys_sage::ComponentType::type _componentType): Component(parent, _id, _name, _componentType) { }
sys_sage::Subdivision::Subdivision(int _id, const std::string &_name): Component(_id, _name, sys_sage::ComponentType::Subdivision) { }
sys_sage::Subdivision::Subdivision(Component * parent, int _id, const std::string &_name): Component(parent, _id, _name, sys_sage::ComponentType::Subdivision) { }

//SVTODO should SubdivisionCategory be settable?
void sys_sage::Subdivision::SetSubdivisionCategory(sys_sage::SubdivisionCategory::type subdivisionCategory){category = subdivisionCategory;}
sys_sage::SubdivisionCategory::type sys_sage::Subdivision::GetSubdivisionCategory() const {return category;}

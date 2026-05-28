#include <sys-sage/Chip.hpp>

sys_sage::Chip::Chip(int _id, const std::string &_name, sys_sage::ChipCategory::type _category, const std::string &_vendor, const std::string &_model):Component(_id, _name, sys_sage::ComponentType::Chip), vendor(_vendor), model(_model), category(_category) {}
sys_sage::Chip::Chip(Component * parent, int _id, const std::string &_name, sys_sage::ChipCategory::type _category, const std::string &_vendor, const std::string &_model):Component(parent, _id, _name, sys_sage::ComponentType::Chip), vendor(_vendor), model(_model), category(_category){}

const std::string& sys_sage::Chip::GetVendor() const{return vendor;}
void sys_sage::Chip::SetVendor(const std::string &_vendor){vendor = _vendor;}
const std::string& sys_sage::Chip::GetModel() const{return model;}
void sys_sage::Chip::SetModel(const std::string &_model){model = _model;}
void sys_sage::Chip::SetChipCategory(sys_sage::ChipCategory::type chipCategory){category = chipCategory;}
sys_sage::ChipCategory::type sys_sage::Chip::GetChipCategory() const{return category;}
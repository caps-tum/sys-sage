#include "Chip.hpp"


sys_sage::Chip::Chip(int _id, std::string _name, int _type, std::string _vendor, std::string _model):Component(_id, _name, sys_sage::ComponentType::Chip), vendor(_vendor), model(_model), type(_type) {}
sys_sage::Chip::Chip(Component * parent, int _id, std::string _name, int _type, std::string _vendor, std::string _model):Component(parent, _id, _name, sys_sage::ComponentType::Chip), vendor(_vendor), model(_model), type(_type){}


const std::string& sys_sage::Chip::GetVendor() const{return vendor;}
void sys_sage::Chip::SetVendor(std::string _vendor){vendor = _vendor;}
const std::string& sys_sage::Chip::GetModel() const{return model;}
void sys_sage::Chip::SetModel(std::string _model){model = _model;}
void sys_sage::Chip::SetChipType(sys_sage::ChipType::type chipType){type = chipType;}
sys_sage::ChipType::type sys_sage::Chip::GetChipType() const{return type;}

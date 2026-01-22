#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>
#include <stdexcept>

#include "Component.hpp"

using std::cout;
using std::endl;

sys_sage::Component * sys_sage::DataPath::GetSource() const {return components[0];}
sys_sage::Component * sys_sage::DataPath::GetTarget() const {return components[1];}
double sys_sage::DataPath::GetBandwidth() const {return bw;}
void sys_sage::DataPath::SetBandwidth(double _bandwidth) { bw = _bandwidth;}
double sys_sage::DataPath::GetLatency() const {return latency;}
void sys_sage::DataPath::SetLatency(double _latency) { latency = _latency; }
sys_sage::DataPathType::type sys_sage::DataPath::GetDataPathType() const {return dp_type;}
sys_sage::DataPathOrientation::type sys_sage::DataPath::GetOrientation() const {return ordered ? sys_sage::DataPathOrientation::Oriented : sys_sage::DataPathOrientation::Bidirectional;}

int sys_sage::DataPath::UpdateSource(Component * _new_source)
{
    return UpdateComponent(0, _new_source);
}

int sys_sage::DataPath::UpdateTarget(Component * _new_target)
{
    return UpdateComponent(1, _new_target);
}

sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type): DataPath(_source, _target, _oriented, _dp_type, -1, -1) {}
sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, double _bw, double _latency): DataPath(_source, _target, _oriented, sys_sage::DataPathType::None, _bw, _latency) {}
sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type, double _bw, double _latency) : Relation(sys_sage::RelationType::DataPath, sys_sage::RelationCategory::Default), dp_type(_dp_type), bw(_bw), latency(_latency)
{
    if(_oriented == sys_sage::DataPathOrientation::Bidirectional)
        ordered = false;
    else
        ordered = true;
    
    AddComponent(_source);
    if (_source != _target)
        AddComponent(_target);
    else
        components.emplace_back(_target);
}

void sys_sage::DataPath::Delete()
{
    Relation::Delete();
}


void sys_sage::DataPath::Print() const
{
    std::cout << GetTypeStr() << " (" << (ordered?"ordered":"unordered") << ")";
    _PrintRelationComponentInfo();
    cout << " -- bw: " << bw << ", latency: " << latency;
    _PrintRelationAttrib();
    cout << endl;
}


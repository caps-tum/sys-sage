#include <sys-sage/QuantumGate.hpp>
#include <iostream>

sys_sage::QuantumGate::QuantumGate(size_t _gate_size, const std::string &_name, double _fidelity, const std::string &_unitary) : Relation(sys_sage::RelationType::QuantumGate, sys_sage::RelationCategory::Default), gate_size(_gate_size), name(_name), fidelity(_fidelity), unitary(_unitary) {}
sys_sage::QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits) : QuantumGate(_gate_size, _qubits, "QuantumGate", 0.0, ""){}
sys_sage::QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, const std::string &_name, double _fidelity, const std::string &_unitary) : Relation(sys_sage::RelationType::QuantumGate, sys_sage::RelationCategory::Default), gate_size(_gate_size), fidelity(_fidelity), unitary(_unitary) 
{ 
    name =_name ;
    for(Qubit* qp : _qubits)
    {
        AddComponent(reinterpret_cast<Component*>(qp));
    }
}
sys_sage::QuantumGate::QuantumGate(const std::vector<Component*>& components, int _id, bool _ordered, size_t _gate_size, const std::string &_name, int _gate_length, QuantumGateCategory::type _gate_category, double _fidelity, const std::string &_unitary) : Relation(components, _id, _ordered, sys_sage::RelationType::QuantumGate, sys_sage::RelationCategory::Default), gate_size(_gate_size), name(_name), gate_length(_gate_length), gate_category(_gate_category), fidelity(_fidelity), unitary(_unitary) {}

void sys_sage::QuantumGate::SetGateProperties(const std::string &_name, double _fidelity, const std::string &_unitary)
{
    name = _name;
    fidelity = _fidelity;
    unitary = _unitary;
    SetQuantumGateCategory();
}

void sys_sage::QuantumGate::SetQuantumGateCategory()
{
    //TODO unite the name in the QuantumGateCategory namespace, similar to RelationType?
    if(gate_size == 1)
    {
        if(name == "id") gate_category = QuantumGateCategory::Id;
        else if(name == "rz") gate_category = QuantumGateCategory::Rz;
        else if(name == "sx") gate_category = QuantumGateCategory::Sx;
        else if(name == "x") gate_category = QuantumGateCategory::X;
        else gate_category = QuantumGateCategory::Unknown;
    }
    else if(gate_size == 2)
    {
        if(name == "cx") gate_category = QuantumGateCategory::Cnot;
        else gate_category = QuantumGateCategory::Unknown;
    }
    else if(gate_size > 2)
    {
        if(name == "toffoli") gate_category = QuantumGateCategory::Toffoli;
        else gate_category = QuantumGateCategory::Unknown;
    }
    else
    {
        gate_category = QuantumGateCategory::Unknown;
    }
}

void sys_sage::QuantumGate::SetName(const std::string &_name)
{
    name = _name;
}

void sys_sage::QuantumGate::SetGateSize(size_t gateSize)
{
    gate_size = gateSize;
}

int sys_sage::QuantumGate::GetGateLength() const
{
    return gate_length;
}

void sys_sage::QuantumGate::SetGateLength(int GateLength)
{
    gate_length = GateLength;
}

void sys_sage::QuantumGate::SetFidelity(double gateFidelity)
{
    fidelity = gateFidelity;
}

void sys_sage::QuantumGate::SetUnitary(const std::string & gateUnitary)
{
    unitary = gateUnitary;
}

sys_sage::QuantumGateCategory::type sys_sage::QuantumGate::GetQuantumGateCategory() const { return gate_category; }
double sys_sage::QuantumGate::GetFidelity() const { return fidelity; }
size_t sys_sage::QuantumGate::GetGateSize() const { return gate_size; }
const std::string& sys_sage::QuantumGate::GetUnitary() const { return unitary; }
const std::string &sys_sage::QuantumGate::GetName() const { return name; }

void sys_sage::QuantumGate::Print() const
{
    std::cout << GetTypeStr() << " (" << (ordered?"ordered":"unordered") << ")";
    _PrintRelationComponentInfo();
    std::cout << " --  Name: " << name << " (category " << QuantumGateCategory::ToString(gate_category) << "), GateSize: " << gate_size << ", GateLength: " << gate_length << ", Fidelity: " << fidelity;
    _PrintRelationAttrib();
    std::cout << std::endl;
}

#include "QuantumBackend.hpp"

#include "Qubit.hpp"



sys_sage::QuantumBackend::QuantumBackend(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::QuantumBackend){}
sys_sage::QuantumBackend::QuantumBackend(Component * _parent, int _id, std::string _name):Component(_parent, _id, _name, sys_sage::ComponentType::QuantumBackend){}

void sys_sage::QuantumBackend::SetNumQubits(int _num_qubits) { num_qubits = _num_qubits; }

int sys_sage::QuantumBackend::GetNumQubits() const { return num_qubits; }

void sys_sage::QuantumBackend::addGate(QuantumGate *_gate)
{
    gate_types.push_back(_gate);
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::GetGatesBySize(size_t _gate_size) const 
{
    return FindGatesBySize(_gate_size);
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::FindGatesBySize(size_t _gate_size) const 
{
    std::vector<QuantumGate*> gates;
    gates.reserve(gate_types.size());
    
    for (QuantumGate * gate : gate_types)
    {
        if(_gate_size == gate->GetGateSize())
            gates.emplace_back(gate);        
    }
    
    return gates;
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::GetGatesByType(QuantumGateType::type _gate_type) const 
{
    return FindGatesByType(_gate_type);
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::FindGatesByType(QuantumGateType::type _gate_type) const 
{
    std::vector<QuantumGate*> gates;
    gates.reserve(gate_types.size());
    
    for (QuantumGate * gate : gate_types)
    {
        if(_gate_type == gate->GetQuantumGateType())
            gates.emplace_back(gate);        
    }
    
    return gates;
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::GetAllGateTypes() const 
{
    return gate_types;
}

size_t sys_sage::QuantumBackend::GetNumberofGates() const { return gate_types.size(); }

std::vector<sys_sage::Qubit *> sys_sage::QuantumBackend::GetAllQubits()
{
    return FindAllQubits();
}

std::vector<sys_sage::Qubit *> sys_sage::QuantumBackend::FindAllQubits()
{
    auto all_children = FindChildrenByType(sys_sage::ComponentType::Qubit);
    std::vector<Qubit *> qubits;
    qubits.reserve(all_children.size());
    
    for (size_t i = 0; i < all_children.size(); ++i)
    {
        Qubit* q = dynamic_cast<Qubit*>(all_children[i]);
        qubits.push_back(q);
    }

    return qubits;
}

#ifdef QDMI
void sys_sage::QuantumBackend::SetQDMIDevice(QDMI_Device dev)
{
    device = dev;
}

QDMI_Device sys_sage::QuantumBackend::GetQDMIDevice(){ return device; }

#endif



#include "CouplingMap.hpp"

#include "Qubit.hpp"

sys_sage::CouplingMap::CouplingMap(Qubit* q1, Qubit* q2) : Relation(sys_sage::RelationType::CouplingMap, sys_sage::RelationCategory::Default)
{
    ordered = true;    
    AddComponent(q1);
    AddComponent(q2);
}
sys_sage::CouplingMap::CouplingMap(const std::vector<Component*>& components, int _id, bool _ordered): Relation(components, _id, _ordered, sys_sage::RelationType::CouplingMap, sys_sage::RelationCategory::Default) {}

void sys_sage::CouplingMap::SetFidelity(double _fidelity){fidelity = _fidelity;}
double sys_sage::CouplingMap::GetFidelity() const {return fidelity;}
void sys_sage::CouplingMap::Delete()
{
    Relation::Delete();
}

#include <sys-sage/CouplingMap.hpp>
#include <sys-sage/Qubit.hpp>
#include <sys-sage/enums.hpp>

sys_sage::CouplingMap::CouplingMap(Qubit* q1, Qubit* q2) : Relation(sys_sage::RelationType::CouplingMap, sys_sage::RelationCategory::Default)
{
    ordered = true;    
    AddComponent(q1);
    AddComponent(q2);
}
sys_sage::CouplingMap::CouplingMap(const std::vector<Component*>& components, int _id, bool _ordered): Relation(components, _id, _ordered, sys_sage::RelationType::CouplingMap, sys_sage::RelationCategory::Default) {}

sys_sage::CouplingMap::CouplingMap(int _id, bool _ordered): Relation(_id, _ordered, sys_sage::RelationType::CouplingMap, sys_sage::RelationCategory::Default) {}

void sys_sage::CouplingMap::SetFidelity(double _fidelity){fidelity = _fidelity;}
double sys_sage::CouplingMap::GetFidelity() const {return fidelity;}

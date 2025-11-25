#include "Component.hpp"

#include "Topology.hpp"  // Needed for sizeof(Topology)
#include "Component.hpp"
#include "Thread.hpp"
#include "Core.hpp"
#include "Cache.hpp"
#include "Subdivision.hpp"
#include "Numa.hpp"
#include "Chip.hpp"
#include "Memory.hpp"
#include "Storage.hpp"
#include "Node.hpp"
#include "QuantumBackend.hpp"
#include "Qubit.hpp"
#include "Relation.hpp"
#include "DataPath.hpp"
#include "QuantumGate.hpp"
#include "CouplingMap.hpp"

#include <algorithm>
#include <csignal>

// Component::~Component() { 
//     DeleteAllDataPaths();
//     if(GetParent() != NULL)
//         GetParent()->RemoveChild(this);
//     else{
//         while(children.size() > 0)
//         {
//             RemoveChild(children[0]);
//             children[0]->SetParent(NULL);
//         }
//     }
//     for(auto& pair : this->attrib){
//         //TODO: delete attribs somehow
//     }
//  }

using std::string;
using std::vector;
using std::cout;
using std::endl;

void sys_sage::Component::PrintSubtree() const { _PrintSubtree(0); }
void sys_sage::Component::_PrintSubtree(int level) const
{
    //TODO maybe print more info based on component type? (override this function?)
    //cout << "---PrintSubtree---" << endl;
    for (int i = 0; i < level; ++i)
        std::cout << "  ";

    cout << GetComponentTypeStr() << " (name " << name << ") id " << id << " - children: " << children.size();
    cout << " level: " << level<<"\n";
    for(Component* child: children)
    {
        //cout << "size of children: " << child->children.size() << "\n";
        child->_PrintSubtree(level + 1);
    }
}
void sys_sage::Component::PrintAllDataPathsInSubtree()
{
    PrintRelationsInSubtree(RelationType::DataPath);
}

void sys_sage::Component::PrintAllRelationsInSubtree(RelationType::type relationType)
{
    PrintRelationsInSubtree(relationType);
}

void sys_sage::Component::PrintRelationsInSubtree(RelationType::type relationType)
{
    vector<Component*> subtreeList;
    FindDescendantsByType(&subtreeList, ComponentType::Any);
    for(Component * c : subtreeList)
    {   
        for(RelationType::type rt : RelationType::RelationTypeList)
        {
            if (relationType == rt || relationType == RelationType::Any)
            {
                vector<Relation*> c_relations = c->GetRelationsByType(rt);
                if(c_relations.size() > 0)
                {
                    std::cout << RelationType::ToString(rt) << "s regarding Component (" << c->GetComponentTypeStr() << ") id " << c->GetId() << std::endl;
                    for(Relation * r : c_relations )
                    {
                        cout << "    ";
                        r->Print();
                    }
                }
            }
        }


    }
}

void sys_sage::Component::InsertChild(Component * child)
{
    child->SetParent(this);
    children.push_back(child);
}
int sys_sage::Component::InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild)
{
    //consistency check
    vector<Component*>& p_children = parent->_GetChildren();
    if(child->GetParent() != parent){
        if(std::find(p_children.begin(), p_children.end(), child) != p_children.end())
            return 1; //child and parent are not child and parent in the component tree
        else
            return 2; //corrupt component tree -> bad thing
    }
    else{
        if(std::find(p_children.begin(), p_children.end(), child) == p_children.end())
            return 3; //corrupt component tree -> bad thing
    }

    //remove from grandparent's list; set new parent; insert child into the new component's list
    p_children.erase(std::remove(p_children.begin(), p_children.end(), child), p_children.end());
    child->SetParent(this);
    this->InsertChild(child);

    //finally, insert new component to grandparent's children list
    if(!alreadyParentsChild)
    {
        this->SetParent(parent);
        parent->InsertChild(this);
    }

    return 0;
}
int sys_sage::Component::InsertBetweenParentAndChildren(Component* parent, std::vector<Component*> children, bool alreadyParentsChild)
{
    vector<Component*>& p_children = parent->_GetChildren();
    for(Component* child: children) //first just check for consistency
    {
        bool isParent = (child->GetParent() == parent);      
        if(std::find(p_children.begin(), p_children.end(), child) == p_children.end()){  //child not listed as parent's child
            if(isParent)
                return 2; //corrupt component tree -> bad thing
            else
                return 1; // just entered a component in the list, which is not a child of the parent
        }
        if(!isParent)
            return 3; //corrupt component tree -> bad thing
    }

    for(Component* child: children) //second time do the actual inserting
    {
        //remove from grandparent's list; set new parent; insert child into the new component's list
        p_children.erase(std::remove(p_children.begin(), p_children.end(), child), p_children.end());
        child->SetParent(this);
        this->InsertChild(child);
    }

    //finally, insert new component to grandparent's children list
    if(!alreadyParentsChild)
    {
        this->SetParent(parent);
        parent->InsertChild(this);
    }
    
    return 0;
}
int sys_sage::Component::RemoveChild(Component * child)
{
    int orig_size = children.size();
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    return orig_size - children.size();
    //return std::erase(children, child); -- not supported in some compilers
}
sys_sage::Component* sys_sage::Component::GetChild(int _id) const
{
    return GetChildById(_id);
}

sys_sage::Component* sys_sage::Component::GetChildById(int _id) const
{
    for(Component* child: children)
    {
        if(child->id == _id)
            return child;
    }
    return NULL;
}
sys_sage::Component* sys_sage::Component::GetChildByType(int _componentType) const
{
    for(Component* child: children)
    {
        if(child->GetComponentType() == _componentType)
            return child;
    }
    return NULL;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetAllChildrenByType(ComponentType::type _componentType) const
{
    vector<Component*> ret;
    FindChildrenByType(&ret, _componentType);
    return ret;
}

std::vector<sys_sage::Component*> sys_sage::Component::FindChildrenByType(ComponentType::type _componentType) const
{
    vector<Component*> ret;
    FindChildrenByType(&ret, _componentType);
    return ret;
}

void sys_sage::Component::GetAllChildrenByType(std::vector <Component *> *_outArray, ComponentType::type _componentType) const
{
    FindChildrenByType(_outArray, _componentType);
}

void sys_sage::Component::FindChildrenByType(std::vector <Component *> *_outArray, ComponentType::type _componentType) const
{
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            _outArray->push_back(child);
    }
    return;
}

int sys_sage::Component::GetSubtreeDepth() const
{
    return CalcSubtreeDepth();
}

int sys_sage::Component::CalcSubtreeDepth() const
{
    if(children.empty()) //is leaf
        return 0;
    int maxDepth = 0;
    for(Component* child: children)
    {
        int subtreeDepth = child->CalcSubtreeDepth();
        if(subtreeDepth > maxDepth)
            maxDepth = subtreeDepth;
    }
    return maxDepth + 1;
}

sys_sage::Component* sys_sage::Component::GetNthAncestor(int n)
{
    // For cases with incorrect inputs (0 and negative values)
    if (n < 0)
        return nullptr; // n can't be negative
    else if(n == 0)
        return this; // n = 0 means "this"
    
    Component* parent = GetParent();
    
    // No parent means no further ancestors, the search can be stopped here.
    if(parent == nullptr)
        return nullptr; 
    
    // base case
    if(n == 1) 
    {
        return parent;
    }
    return parent->GetNthAncestor(n - 1);
        
}

void sys_sage::Component::GetNthDescendents(std::vector<Component*>* outArray, int depth)
{
    FindNthDescendants(outArray, depth);
}

void sys_sage::Component::FindNthDescendants(std::vector<Component*>* outArray, int depth)
{
    
    if(depth <= 0)
    {   
        outArray->push_back(this);
        // depth++;
        return;
    }
    for(Component* child: children)
    {   
        cout << GetComponentTypeStr() << " (name " << name << ") id " << id << " - children: " << children.size();
        cout << " depth: " << depth<<"\n";
        child->FindNthDescendants(outArray, depth - 1);
    }
    return;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetNthDescendents(int depth)
{
    vector<Component*> outArray;
    FindNthDescendants(&outArray, depth);
    return outArray;
}

std::vector<sys_sage::Component*> sys_sage::Component::FindNthDescendants(int depth)
{
    vector<Component*> outArray;
    FindNthDescendants(&outArray, depth);
    return outArray;
}

void sys_sage::Component::GetSubcomponentsByType(std::vector<Component*>* outArray, sys_sage::ComponentType::type _componentType)
{
    FindDescendantsByType(outArray, _componentType);
}

std::vector<sys_sage::Component*> sys_sage::Component::GetSubcomponentsByType(sys_sage::ComponentType::type _componentType)
{
    vector<Component*> ret;
    FindDescendantsByType(&ret, _componentType);
    return ret;
}

void sys_sage::Component::GetComponentsInSubtree(std::vector<Component*>* outArray)
{
    FindDescendantsByType(outArray, ComponentType::Any);
}

std::vector<sys_sage::Component*> sys_sage::Component::GetComponentsInSubtree()
{
    vector<Component*> outArray;
    FindDescendantsByType(&outArray, ComponentType::Any);
    return outArray;
}

sys_sage::Component* sys_sage::Component::GetSubcomponentById(int _id, ComponentType::type _componentType)
{
    return GetDescendantById(_id, _componentType);
}

sys_sage::Component *sys_sage::Component::GetDescendantById(int _id, ComponentType::type _componentType)
{
    if(componentType == _componentType && id == _id){
        return this;
    }
    for(Component * child : children)
    {
        Component* ret = child->GetDescendantById(_id, _componentType);
        if(ret != NULL)
        {
            return ret;
        }
    }
    return NULL;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetAllSubcomponentsByType(ComponentType::type _componentType)
{
    vector<Component*> ret;
    FindDescendantsByType(&ret, _componentType);
    return ret;
}

std::vector<sys_sage::Component*> sys_sage::Component::FindDescendantsByType(ComponentType::type _componentType)
{
    vector<Component*> ret;
    FindDescendantsByType(&ret, _componentType);
    return ret;
}

void sys_sage::Component::GetAllSubcomponentsByType(std::vector<Component*>* outArray, ComponentType::type _componentType)
{
    FindDescendantsByType(outArray, _componentType);
}

void sys_sage::Component::FindDescendantsByType(std::vector<Component*>* outArray, ComponentType::type _componentType)
{
    if(_componentType == ComponentType::Any || componentType == _componentType){
        outArray->push_back(this);
    }
    for(Component * child : children)
    {
        child->FindDescendantsByType(outArray, _componentType);
    }
    return;
}

int sys_sage::Component::CountAllSubcomponents() const
{
    return CountDescendantsByType(ComponentType::Any);
}

int sys_sage::Component::CountAllSubcomponentsByType(ComponentType::type _componentType) const
{
    return CountDescendantsByType(_componentType);
}

int sys_sage::Component::CountDescendantsByType(ComponentType::type _componentType) const
{
    int cnt = 0;
    for(Component * child : children)
    {
        if(_componentType == ComponentType::Any || child->GetComponentType() == _componentType)
            cnt++;
    }
    for(Component * child : children)
    {
        cnt += child->CountDescendantsByType(_componentType);
    }
    return cnt;
}

int sys_sage::Component::CountAllChildrenByType(ComponentType::type _componentType) const
{
    return CountChildrenByType(_componentType);
}

int sys_sage::Component::CountChildrenByType(ComponentType::type _componentType) const
{
    int cnt = 0;
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            cnt++;
    }

    return cnt;
}

sys_sage::Component* sys_sage::Component::GetAncestorByType(ComponentType::type _componentType)
{
    if(componentType == _componentType){
        return this;
    }
    if(parent != NULL){
        return parent->GetAncestorByType(_componentType);
    }
    return NULL;
}

void sys_sage::Component::_AddRelation(RelationType::type relationType, Relation* r)
{
    if(!relations)
        relations = new std::array<std::vector<Relation*>*, RelationType::_num_relation_types>();
    if(!(*relations)[relationType])
        (*relations)[relationType] = new std::vector<Relation*>();
    
    (*relations)[relationType]->push_back(r);
}

sys_sage::DataPath* sys_sage::Component::GetDataPathByType(DataPathType::type  dp_type, DataPathDirection::type direction) const
{
    for(Relation* r: *(*relations)[RelationType::DataPath])
    {
        //either unordered -> check; or orientation is any -> check; or orientation is incoming & DP is incoming or the same outgoing
        if(!r->IsOrdered() || 
            direction == DataPathDirection::Any || 
            (direction == DataPathDirection::Outgoing && r->GetComponent(0) == this) ||  
            (direction == DataPathDirection::Incoming && r->GetComponent(1) == this))
        {
            DataPath* dp = reinterpret_cast<DataPath*>(r);
            if(dp->GetDataPathType() == dp_type)
                return dp;
        }
    }
    return NULL;
}

std::vector<sys_sage::Relation*>& sys_sage::Component::_GetRelations(RelationType::type relationType) const
{
    return _GetRelationsByType(relationType);
}

std::vector<sys_sage::Relation*>& sys_sage::Component::_GetRelationsByType(RelationType::type relationType) const
{
    if (relations &&
        relationType >= 0 && 
        relationType < RelationType::_num_relation_types &&
        (*relations)[relationType]) 
    {
        return *(*relations)[relationType];
    }

    static std::vector<Relation*> empty;
    return empty;
}

const std::vector<sys_sage::Relation*>& sys_sage::Component::GetRelations(RelationType::type relationType) const
{
    return GetRelationsByType(relationType);
}

const std::vector<sys_sage::Relation*>& sys_sage::Component::GetRelationsByType(RelationType::type relationType) const
{
    if (relations &&
        relationType >= 0 && 
        relationType < RelationType::_num_relation_types &&
        (*relations)[relationType]) 
    {
        return *(*relations)[relationType];
    }

    static const std::vector<Relation*> empty;
    return empty;
}

std::vector<sys_sage::Relation*> sys_sage::Component::GetAllRelationsBy(RelationType::type relationType, int thisComponentPosition) const
{
    return FindRelations(relationType, thisComponentPosition);
}

std::vector<sys_sage::Relation*> sys_sage::Component::FindRelations(RelationType::type relationType, int thisComponentPosition) const
{
    vector<Relation*> out_vector;
    for(int curr_rt : RelationType::RelationTypeList)
    {
        if(relationType == RelationType::Any || relationType == curr_rt)
        {
            for(Relation* r : *(*relations)[curr_rt])
            {
                if(!r->IsOrdered() || (r->IsOrdered() && (thisComponentPosition==-1 || r->GetComponent(thisComponentPosition) == this)))
                {
                    out_vector.push_back(r);
                }
            }
        }
    }
    return out_vector;
}

void sys_sage::Component::GetAllDataPaths(std::vector<DataPath*>* outDpArr, DataPathType::type dp_type, DataPathDirection::type direction) const
{
    FindDataPaths(outDpArr, dp_type, direction);
}

void sys_sage::Component::FindDataPaths(std::vector<DataPath*>* outDpArr, DataPathType::type dp_type, DataPathDirection::type direction) const
{
    if (relations == nullptr || (*relations)[RelationType::DataPath] == nullptr)
        return;

    for(Relation* r: *(*relations)[RelationType::DataPath])
    {
        //either unordered -> check; or orientation is any -> check; or orientation is incoming & DP is incoming or the same outgoing
        if(!r->IsOrdered() || 
            direction == DataPathDirection::Any || 
            (direction == DataPathDirection::Outgoing && r->GetComponent(0) == this) ||
            (direction == DataPathDirection::Incoming && r->GetComponent(1) == this))
        {
            DataPath* dp = reinterpret_cast<DataPath*>(r);
            if(dp_type == DataPathType::Any || dp->GetDataPathType() == dp_type)
                outDpArr->push_back(dp);
        }
    }
    return;
}

std::vector<sys_sage::DataPath*> sys_sage::Component::GetAllDataPaths(DataPathType::type dp_type, DataPathDirection::type direction) const
{
    vector<DataPath*> outDpArr;
    FindDataPaths(&outDpArr, dp_type, direction);
    return outDpArr;
}

std::vector<sys_sage::DataPath*> sys_sage::Component::FindDataPaths(DataPathType::type dp_type, DataPathDirection::type direction) const
{
    vector<DataPath*> outDpArr;
    FindDataPaths(&outDpArr, dp_type, direction);
    return outDpArr;
}

std::string sys_sage::Component::GetComponentTypeStr() const
{
    std::string ret(ComponentType::ToString(componentType));
    return ret;
}

int sys_sage::Component::CheckComponentTreeConsistency() const
{
    return CheckSubtreeConsistency();
}

int sys_sage::Component::CheckSubtreeConsistency() const
{
    int errors = 0;
    for(Component * child : children){
        if(child->GetParent() != this){
            // std::cerr << "Component " << child->GetComponentTypeStr() << " id " << child->GetId() << " has wrong parent" << std::endl;
            errors++;
        }
    }
    for(Component * child : children){
        errors += child->CheckSubtreeConsistency();
    }
    return errors;
}

int sys_sage::Component::GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize) const
{
    std::set<Relation *> countedRelations;
    return _CalcSubtreeSize(out_component_size, out_RelationSize, &countedRelations);
}

int sys_sage::Component::CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize) const
{
    std::set<Relation *> countedRelations;
    return _CalcSubtreeSize(out_component_size, out_RelationSize, &countedRelations);
}

int sys_sage::Component::_GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const
{
    return _CalcSubtreeSize(out_component_size, out_RelationSize, countedRelations);
}

int sys_sage::Component::_CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const
{
    int component_size = 0;
    switch(componentType)
    {
        case ComponentType::Generic:
            component_size += sizeof(Component);
        break;
        case ComponentType::Thread:
            component_size += sizeof(Thread);
        break;
        case ComponentType::Core:
            component_size += sizeof(Core);
        break;
        case ComponentType::Cache:
            component_size += sizeof(Cache);
        break;
        case ComponentType::Subdivision:
            component_size += sizeof(Subdivision);
        break;
        case ComponentType::Numa:
            component_size += sizeof(Numa);
        break;
        case ComponentType::Chip:
            component_size += sizeof(Chip);
        break;
        case ComponentType::Memory:
            component_size += sizeof(Memory);
        break;
        case ComponentType::Storage:
            component_size += sizeof(Storage);
        break;
        case ComponentType::Node:
            component_size += sizeof(Node);
        break;
        case ComponentType::Topology:
            component_size += sizeof(Topology);
        break;
    }
    component_size += attrib.size()*(sizeof(std::string)+sizeof(void*)); //TODO improve
    component_size += children.size()*sizeof(Component*);
    //relations -- only counting the vector/array sizes
    if(relations)
    {
        component_size += sizeof(std::array<std::vector<Relation*>*, RelationType::_num_relation_types>);
        for(int i = 0; i<RelationType::_num_relation_types; i++)
        {
            if((*relations)[i] != NULL)
            {
                component_size += sizeof(*(*relations)[i]);
            }
        }
    }
    (*out_component_size) += component_size;

    int relationsSize = 0;


    for(RelationType::type rt : RelationType::RelationTypeList)
    {
        std::vector<Relation*> rv = GetRelationsByType(rt);
        for(Relation* r: rv)
        {
            if(countedRelations->find(r) == countedRelations->end())
            {
                switch(rt)
                {
                    case RelationType::Relation:
                        relationsSize += sizeof(Relation);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                    case RelationType::DataPath:
                        relationsSize += sizeof(DataPath);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                    case RelationType::QuantumGate:
                        relationsSize += sizeof(QuantumGate);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                    case RelationType::CouplingMap:
                        relationsSize += sizeof(CouplingMap);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                }
                countedRelations->insert(r);
            }
        }
    }
    (*out_RelationSize) += relationsSize;

    int subtreeSize = 0;
    for(Component * c : children)
    {
        subtreeSize += c->_CalcSubtreeSize(out_component_size, out_RelationSize, countedRelations);
    }

    return component_size + relationsSize + subtreeSize;
}

int sys_sage::Component::GetDepth(bool refresh)
{
    return CalcDepth(refresh);
}

int sys_sage::Component::CalcDepth(bool refresh)
{
    if(refresh)
    {
        depth = 0;
        Component *parent = GetParent();
        while(parent != NULL)
        {
            depth++;
            parent = parent->GetParent();
        }
    }
    
    return depth;
}

void sys_sage::Component::DeleteRelation(Relation * r)
{
    int32_t rt = r->GetType();
    if(rt == RelationType::Relation)
        r->Delete();
    else if(rt == RelationType::DataPath){
        DataPath* dp = reinterpret_cast<DataPath*>(r);
        dp->Delete();
    } else if(rt == RelationType::QuantumGate){
        QuantumGate* qg = reinterpret_cast<QuantumGate*>(r);
        qg->Delete();
    } else if(rt == RelationType::CouplingMap){
        CouplingMap* cm = reinterpret_cast<CouplingMap*>(r);
        cm->Delete();
    } else{ //this should never happen
        std::cout << "ERROR void sys_sage::Component::DeleteRelation(Relation * r)" << std::endl;
        exit(1);
    }    
}

void sys_sage::Component::DeleteAllRelations(RelationType::type relationType)
{
    DeleteRelations(relationType);
}

void sys_sage::Component::DeleteRelations(RelationType::type relationType)
{
    for(RelationType::type rt : RelationType::RelationTypeList)
    {
        if(relationType == RelationType::Any || relationType == rt)
        {
            while(true)
            {
                vector<Relation*> vec_r = GetRelationsByType(rt);
                if(vec_r.size() > 0)
                {
                    DeleteRelation(vec_r[0]);
                }
                else
                    break;
            }
        }
    }
}


void sys_sage::Component::DeleteDataPath(DataPath * dp)
{
    DeleteRelation(dp);
}

void sys_sage::Component::DeleteAllDataPaths()
{
    DeleteRelations(RelationType::DataPath);
}

void sys_sage::Component::DeleteSubtree() const
{
    while(children.size() > 0)
    {       
        children[0]->Delete(true); // Recursively free children
    }
    return;
}

void sys_sage::Component::Delete(bool withSubtree)
{
    // Delete subtree and all data paths
    if (withSubtree)
    {
        DeleteSubtree();
    }

    DeleteRelations();
    
    //Free all the children
    if(GetParent()!= NULL) 
    {
        Component *myParent = GetParent();
        myParent->RemoveChild(this);
        if (!withSubtree)
        {
            for(Component* child: children)
            {   
                child->SetParent(myParent);
                myParent->InsertChild(child);
            }
        }    
    }
    else //if(GetParent() == NULL && !withSubtree)
    {
        while(children.size() > 0)
        {       
            RemoveChild(children[0]); // Recursively free children
            children[0]->SetParent(NULL);
        }
    }
    // Delete the component itself
    delete this;
}

const std::string& sys_sage::Component::GetName() const {return name;}
void sys_sage::Component::SetName(std::string _name){ name = _name; }
sys_sage::Component* sys_sage::Component::GetParent() const {return parent;}
void sys_sage::Component::SetParent(Component* _parent){parent = _parent;}
const std::vector<sys_sage::Component*>& sys_sage::Component::GetChildren() const {return children;}
std::vector<sys_sage::Component*>& sys_sage::Component::_GetChildren() {return children;}
sys_sage::ComponentType::type sys_sage::Component::GetComponentType() const {return componentType;}
int sys_sage::Component::GetId() const {return id;}

sys_sage::Component::Component(int _id, std::string _name, ComponentType::type _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(NULL);
}
sys_sage::Component::Component(Component * parent, int _id, std::string _name, ComponentType::type _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(parent);
    if (parent) {
        parent->InsertChild(this);
    }
}
sys_sage::Component::Component(int _id, std::string _name): Component(_id, _name, sys_sage::ComponentType::Generic) {}
sys_sage::Component::Component(Component * parent, int _id, std::string _name): Component(parent, _id, _name, sys_sage::ComponentType::Generic) {}

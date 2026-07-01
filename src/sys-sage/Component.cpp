#include <sys-sage/Component.hpp>

#include <sys-sage/Topology.hpp>  // Needed for sizeof(Topology)
#include <sys-sage/Thread.hpp>
#include <sys-sage/Core.hpp>
#include <sys-sage/Cache.hpp>
#include <sys-sage/Subdivision.hpp>
#include <sys-sage/Numa.hpp>
#include <sys-sage/Chip.hpp>
#include <sys-sage/Memory.hpp>
#include <sys-sage/Storage.hpp>
#include <sys-sage/Node.hpp>
#include <sys-sage/QuantumBackend.hpp>
#include <sys-sage/Qubit.hpp>
#include <sys-sage/Relation.hpp>
#include <sys-sage/DataPath.hpp>
#include <sys-sage/QuantumGate.hpp>
#include <sys-sage/CouplingMap.hpp>

#include <algorithm>
#include <csignal>

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

void sys_sage::Component::PrintRelationsInSubtree(RelationType::type relationType)
{
    vector<Component*> subtreeList;
    FindDescendantsByType(subtreeList, ComponentType::Any);
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
    child->SetParent(nullptr);
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

std::vector<sys_sage::Component*> sys_sage::Component::FindChildrenByType(ComponentType::type _componentType) const
{
    vector<Component*> ret;
    FindChildrenByType(ret, _componentType);
    return ret;
}

void sys_sage::Component::FindChildrenByType(std::vector <Component *> &_outArray, ComponentType::type _componentType) const
{
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            _outArray.push_back(child);
    }
    return;
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

void sys_sage::Component::FindNthDescendants(std::vector<Component*> &outArray, int depth)
{
    
    if(depth <= 0)
    {   
        outArray.push_back(this);
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

std::vector<sys_sage::Component*> sys_sage::Component::FindNthDescendants(int depth)
{
    vector<Component*> outArray;
    FindNthDescendants(outArray, depth);
    return outArray;
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

std::vector<sys_sage::Component*> sys_sage::Component::FindDescendantsByType(ComponentType::type _componentType)
{
    vector<Component*> ret;
    FindDescendantsByType(ret, _componentType);
    return ret;
}

void sys_sage::Component::FindDescendantsByType(std::vector<Component*> &outArray, ComponentType::type _componentType)
{
    if(_componentType == ComponentType::Any || componentType == _componentType){
        outArray.push_back(this);
    }
    for(Component * child : children)
    {
        child->FindDescendantsByType(outArray, _componentType);
    }
    return;
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

sys_sage::DataPath* sys_sage::Component::GetDataPathByCategory(DataPathCategory::type  dp_category, DataPathDirection::type direction) const
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
            if(dp->GetDataPathCategory() == dp_category)
                return dp;
        }
    }
    return NULL;
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

void sys_sage::Component::FindDataPaths(std::vector<DataPath*> &outDpArr, DataPathCategory::type dp_category, DataPathDirection::type direction) const
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
            if(dp_category == DataPathCategory::Any || dp->GetDataPathCategory() == dp_category)
                outDpArr.push_back(dp);
        }
    }
    return;
}

std::vector<sys_sage::DataPath*> sys_sage::Component::FindDataPaths(DataPathCategory::type dp_category, DataPathDirection::type direction) const
{
    vector<DataPath*> outDpArr;
    FindDataPaths(outDpArr, dp_category, direction);
    return outDpArr;
}

const std::string &sys_sage::Component::GetComponentTypeStr() const
{
    //std::string ret(ComponentType::ToString(componentType));
    //return ret;
    return ComponentType::ToString(componentType);
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

int sys_sage::Component::CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize) const
{
    std::set<Relation *> countedRelations;
    return _CalcSubtreeSize(out_component_size, out_RelationSize, countedRelations);
}

// TODO: fix this
int sys_sage::Component::_CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*> &countedRelations) const
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
    // Only consider the keys of the attributes. We can not measure the size of
    // arbitrary values. Maybe remove the size measurement of the attributes or
    // remove this function entirely?
    component_size += sizeof(attributes) + attributes.size() * sizeof(std::string);
    component_size += sizeof(children) + children.size()*sizeof(Component*);
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
            if(countedRelations.find(r) == countedRelations.end())
            {

                relationsSize += r->GetAttributesSize() * sizeof(std::string);
                switch(rt)
                {
                    case RelationType::Relation:
                        relationsSize += sizeof(Relation);
                        break;
                    case RelationType::DataPath:
                        relationsSize += sizeof(DataPath);
                        break;
                    case RelationType::QuantumGate:
                        relationsSize += sizeof(QuantumGate);
                        break;
                    case RelationType::CouplingMap:
                        relationsSize += sizeof(CouplingMap);
                        break;
                }
                countedRelations.insert(r);
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

void sys_sage::Component::DeleteRelations(RelationType::type relationType)
{
    if (!relations) // no relations
        return;

    for(RelationType::type rt : RelationType::RelationTypeList)
    {
        if(relationType == RelationType::Any || relationType == rt)
        {
            std::vector<Relation *> *rel = (*relations)[rt];
            if(rel) // no relation of that specific type
            {
                while(rel->size() > 0)
                {
                    Relation::Delete(rel->back()); // it might be faster to remove from behind
                }

                delete rel;
                (*relations)[rt] = nullptr; // mark as deleted
            }
        }
    }
}

sys_sage::Component::~Component()
{
    if (parent)
        parent->RemoveChild(this);
    for (auto child : children)
        child->SetParent(nullptr);
}

void sys_sage::Component::Delete(Component *comp)
{
    if (comp->relations) {
        comp->DeleteRelations();
        delete comp->relations;
    }

    delete comp;
}

static void HelperDeleteSubtree(sys_sage::Component *comp)
{
    auto &children = comp->GetChildren();
    while (children.size() > 0)
        HelperDeleteSubtree(children.back());
    sys_sage::Component::Delete(comp);
}

void sys_sage::Component::DeleteSubtree(Component *root, bool keepRoot)
{
    while (root->children.size() > 0)
        HelperDeleteSubtree(root->children.back());

    if (!keepRoot)
        Component::Delete(root);
}

const std::string& sys_sage::Component::GetName() const {return name;}
void sys_sage::Component::SetName(const std::string &_name){ name = _name; }
sys_sage::Component* sys_sage::Component::GetParent() const {return parent;}
void sys_sage::Component::SetParent(Component* _parent){parent = _parent;}
const std::vector<sys_sage::Component*>& sys_sage::Component::GetChildren() const {return children;}
std::vector<sys_sage::Component*>& sys_sage::Component::_GetChildren() {return children;}
sys_sage::ComponentType::type sys_sage::Component::GetComponentType() const {return componentType;}
int sys_sage::Component::GetId() const {return id;}
void sys_sage::Component::SetId(int _id) { id = _id; }

sys_sage::Component::Component(int _id, const std::string &_name, ComponentType::type _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(NULL);
}
sys_sage::Component::Component(Component * parent, int _id, const std::string &_name, ComponentType::type _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(parent);
    if (parent) {
        parent->InsertChild(this);
    }
}
sys_sage::Component::Component(int _id, const std::string &_name): Component(_id, _name, sys_sage::ComponentType::Generic) {}
sys_sage::Component::Component(Component * parent, int _id, const std::string &_name): Component(parent, _id, _name, sys_sage::ComponentType::Generic) {}

sys_sage::Component::attribSizeType sys_sage::Component::GetAttributesSize() const
{
    return attributes.size();
}

sys_sage::Component::attribIterator sys_sage::Component::AttributesBegin()
{
    return attributes.begin();
}

sys_sage::Component::constAttribIterator sys_sage::Component::AttributesBegin() const
{
    return attributes.begin();
}

sys_sage::Component::attribIterator sys_sage::Component::AttributesEnd()
{
    return attributes.end();
}

sys_sage::Component::constAttribIterator sys_sage::Component::AttributesEnd() const
{
    return attributes.end();
}

void sys_sage::Component::EraseAttribute(const std::string &key)
{
    attributes.erase(key);
}

sys_sage::Component::attribIterator sys_sage::Component::EraseAttribute(Component::attribIterator it)
{
    return attributes.erase(it);
}

void sys_sage::Component::ClearAttributes()
{
    attributes.clear();
}

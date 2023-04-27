#include "Topology.hpp"

#include <algorithm>

void Component::PrintSubtree() { PrintSubtree(0); }
void Component::PrintSubtree(int level)
{
    //cout << "---PrintSubtree---" << endl;
    for (int i = 0; i < level; ++i)
        cout << "  " ;
    cout << GetComponentTypeStr() << " (name " << name << ") id " << id << " - children: " << children.size() <<  endl;

    for(Component* child: children)
    {
        child->PrintSubtree(level+1);
    }
}
void Component::PrintAllDataPathsInSubtree()
{
    vector<Component*> subtreeList;
    GetSubtreeNodeList(&subtreeList);
    for(Component * c : subtreeList)
    {
        vector<DataPath*>* dp_in = c->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
        vector<DataPath*>* dp_out = c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        if(dp_in->size() > 0 || dp_out->size() > 0 )
        {
            cout << "DataPaths regarding Component (" << c->GetComponentTypeStr() << ") id " << c->GetId() << endl;
            for(DataPath * dp : *dp_out)
            {
                cout << "    ";
                dp->Print();
            }
            for(DataPath * dp : *dp_in)
            {
                cout << "    ";
                dp->Print();
            }
        }
    }
}

void Component::InsertChild(Component * child)
{
    child->SetParent(this);
    children.push_back(child);
}
int Component::RemoveChild(Component * child)
{
    int orig_size = children.size();
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    return orig_size - children.size();
    //return std::erase(children, child); -- not supported in some compilers
}
Component* Component::GetChild(int _id)
{
    for(Component* child: children)
    {
        if(child->id == _id)
            return child;
    }
    return NULL;
}
Component* Component::GetChildByType(int _componentType)
{
    for(Component* child: children)
    {
        if(child->GetComponentType() == _componentType)
            return child;
    }
    return NULL;
}

vector<Component*> Component::GetAllChildrenByType(int _componentType)
{
    vector<Component*> ret;
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
         ret.push_back(child);
    }
    return ret;
}

int Component::GetNumThreads()
{
    if(componentType == SYS_SAGE_COMPONENT_THREAD)
        return 1;
    int numPu = 0;
    for(Component* child: children)
    {
        numPu+=child->GetNumThreads();
    }
    return numPu;
}

int Component::GetTopoTreeDepth()
{
    if(children.empty()) //is leaf
        return 0;
    int maxDepth = 0;
    for(Component* child: children)
    {
        int subtreeDepth = child->GetTopoTreeDepth();
        if(subtreeDepth > maxDepth)
            maxDepth = subtreeDepth;
    }
    return maxDepth+1;
}

void Component::GetComponentsNLevelsDeeper(vector<Component*>* outArray, int depth)
{
    if(depth <= 0)
    {
        outArray->push_back(this);
        return;
    }
    for(Component* child: children)
    {
        child->GetComponentsNLevelsDeeper(outArray, depth-1);
    }
    return;
}

void Component::GetSubcomponentsByType(vector<Component*>* outArray, int _componentType)
{
    if(_componentType == componentType)
    {
        outArray->push_back(this);
    }
    for(Component* child: children)
    {
        child->GetSubcomponentsByType(outArray, _componentType);
    }
}

void Component::GetSubtreeNodeList(vector<Component*>* outArray)
{
    outArray->push_back(this);
    for(Component * child : children)
    {
        child->GetSubtreeNodeList(outArray);
    }
    return;
}

Component* Component::FindSubcomponentById(int _id, int _componentType)
{
    if(componentType == _componentType && id == _id){
        return this;
    }
    for(Component * child : children)
    {
        Component* ret = child->FindSubcomponentById(_id, _componentType);
        if(ret != NULL)
        {
            return ret;
        }
    }
    return NULL;
}

void Component::FindAllSubcomponentsByType(vector<Component*>* outArray, int _componentType)
{
    if(componentType == _componentType){
        outArray->push_back(this);
    }
    for(Component * child : children)
    {
        child->FindAllSubcomponentsByType(outArray, _componentType);
    }
    return;
}

Component* Component::FindParentByType(int _componentType)
{
    if(componentType == _componentType){
        return this;
    }
    if(parent != NULL){
        //cout << "   passing through component " << GetId() << " " << GetComponentType() << "(" << GetComponentTypeStr() << ") - searching for " << _componentType << endl;
        return parent->FindParentByType(_componentType);
    }
    return NULL;
}

void Component::AddDataPath(DataPath* p, int orientation)
{
    if(orientation == SYS_SAGE_DATAPATH_OUTGOING)
        dp_outgoing.push_back(p);
    else if(orientation == SYS_SAGE_DATAPATH_INCOMING)
        dp_incoming.push_back(p);
}

DataPath* Component::GetDpByType(int dp_type, int orientation)
{
    if(orientation & SYS_SAGE_DATAPATH_OUTGOING){
        for(DataPath* dp : dp_outgoing){
            if(dp->GetDpType() == dp_type)
                return dp;
        }
    }
    if(orientation & SYS_SAGE_DATAPATH_INCOMING){
        for(DataPath* dp : dp_incoming){
            if(dp->GetDpType() == dp_type)
                return dp;
        }
    }
    return NULL;
}
void Component::GetAllDpByType(vector<DataPath*>* outDpArr, int dp_type, int orientation)
{
    if(orientation & SYS_SAGE_DATAPATH_OUTGOING){
        for(DataPath* dp : dp_outgoing){
            if(dp->GetDpType() == dp_type)
                outDpArr->push_back(dp);
        }
    }
    if(orientation & SYS_SAGE_DATAPATH_INCOMING){
        for(DataPath* dp : dp_incoming){
            if(dp->GetDpType() == dp_type)
                outDpArr->push_back(dp);
        }
    }
    return;
}

vector<DataPath*>* Component::GetDataPaths(int orientation)
{
    if(orientation == SYS_SAGE_DATAPATH_INCOMING)
        return &dp_incoming;
    else if(orientation == SYS_SAGE_DATAPATH_OUTGOING)
        return &dp_outgoing;
    else //TODO
        return NULL;
}

string Component::GetComponentTypeStr()
{
    switch(componentType)
    {
        case SYS_SAGE_COMPONENT_NONE:
            return "None";
        case SYS_SAGE_COMPONENT_THREAD:
            return "HW_thread";
        case SYS_SAGE_COMPONENT_CORE:
            return "Core";
        case SYS_SAGE_COMPONENT_CACHE:
            return "Cache";
        case SYS_SAGE_COMPONENT_SUBDIVISION:
            return "Subdivision";
        case SYS_SAGE_COMPONENT_NUMA:
            return "NUMA";
        case SYS_SAGE_COMPONENT_CHIP:
            return "Chip";
        case SYS_SAGE_COMPONENT_MEMORY:
            return "Memory";
        case SYS_SAGE_COMPONENT_STORAGE:
            return "Storage";
        case SYS_SAGE_COMPONENT_NODE:
            return "Node";
        case SYS_SAGE_COMPONENT_TOPOLOGY:
            return "Topology";
    }
    return "";
}

int Component::CheckComponentTreeConsistency()
{
    int errors = 0;
    for(Component * child : children){
        if(child->GetParent() != this){
            std::cerr << "Component " << child->GetComponentType() << " id " << child->GetName() << "has wrong parent" << std::endl;
            errors++;
        }
    }
    for(Component * child : children){
        errors += child->CheckComponentTreeConsistency();
    }
    return errors;
}

int Component::GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize)
{
    return GetTopologySize(out_component_size, out_dataPathSize, NULL);
}
int Component::GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize, std::set<DataPath*>* counted_dataPaths)
{
    if(counted_dataPaths == NULL)
        counted_dataPaths = new std::set<DataPath*>();

    int component_size = 0;
    switch(componentType)
    {
        case SYS_SAGE_COMPONENT_NONE:
        break;
        case SYS_SAGE_COMPONENT_THREAD:
        component_size += sizeof(Thread);
        break;
        case SYS_SAGE_COMPONENT_CORE:
        component_size += sizeof(Core);
        break;
        case SYS_SAGE_COMPONENT_CACHE:
        component_size += sizeof(Cache);
        break;
        case SYS_SAGE_COMPONENT_SUBDIVISION:
        component_size += sizeof(Subdivision);
        break;
        case SYS_SAGE_COMPONENT_NUMA:
        component_size += sizeof(Numa);
        break;
        case SYS_SAGE_COMPONENT_CHIP:
        component_size += sizeof(Chip);
        break;
        case SYS_SAGE_COMPONENT_MEMORY:
        component_size += sizeof(Memory);
        break;
        case SYS_SAGE_COMPONENT_STORAGE:
        component_size += sizeof(Storage);
        break;
        case SYS_SAGE_COMPONENT_NODE:
        component_size += sizeof(Node);
        break;
        case SYS_SAGE_COMPONENT_TOPOLOGY:
        component_size += sizeof(Topology);
        break;
    }
    component_size += attrib.size()*(sizeof(string)+sizeof(void*)); //TODO improve
    component_size += children.size()*sizeof(Component*);
    (*out_component_size) += component_size;

    int dataPathSize = 0;
    dataPathSize += dp_incoming.size()*sizeof(DataPath*);
    dataPathSize += dp_outgoing.size()*sizeof(DataPath*);
    for(auto it = std::begin(dp_incoming); it != std::end(dp_incoming); ++it) {
        if(!counted_dataPaths->count((DataPath*)(*it))) {
            //cout << "new datapath " << (DataPath*)(*it) << endl;
            dataPathSize += sizeof(DataPath);
            dataPathSize += (*it)->attrib.size()*(sizeof(string)+sizeof(void*)); //TODO improve
            counted_dataPaths->insert((DataPath*)(*it));
        }
    }
    for(auto it = std::begin(dp_outgoing); it != std::end(dp_outgoing); ++it) {
        if(!counted_dataPaths->count((DataPath*)(*it))){
            //cout << "new datapath " << (DataPath*)(*it) << endl;
            dataPathSize += sizeof(DataPath);
            dataPathSize += (*it)->attrib.size()*(sizeof(string)+sizeof(void*)); //TODO improve
            counted_dataPaths->insert((DataPath*)(*it));
        }
    }
    (*out_dataPathSize) += dataPathSize;

    int subtreeSize = 0;
    for(auto it = std::begin(children); it != std::end(children); ++it) {
        subtreeSize += (*it)->GetTopologySize(out_component_size, out_dataPathSize, counted_dataPaths);
    }

    return component_size + dataPathSize + subtreeSize;
}

Component* Component::GetParent(){return parent;}
vector<Component*>* Component::GetChildren(){return &children;}
int Component::GetComponentType(){return componentType;}

string Component::GetName(){return name;}
int Component::GetId(){return id;}

void Component::SetParent(Component* _parent){parent = _parent;}

string Chip::GetVendor(){return vendor;}
void Chip::SetVendor(string _vendor){vendor = _vendor;}
string Chip::GetModel(){return model;}
void Chip::SetModel(string _model){model = _model;}
void Chip::SetChipType(int chipType){type = chipType;}
int Chip::GetChipType(){return type;}

void Subdivision::SetSubdivisionType(int subdivisionType) {type = subdivisionType;}
int Subdivision::GetSubdivisionType() {return type;}

long long Numa::GetSize(){return size;}

long long Memory::GetSize() {return size;}
void Memory::SetSize(long long _size) {size = _size;}

string Cache::GetCacheName(){return cache_type;}
int Cache::GetCacheLevel(){return stoi(cache_type.empty()?"0":cache_type);}
long long Cache::GetCacheSize(){return cache_size;}
void Cache::SetCacheSize(long long _cache_size){cache_size = _cache_size;}
int Cache::GetCacheLineSize(){return cache_line_size;}
void Cache::SetCacheLineSize(int _cache_line_size){cache_line_size = _cache_line_size;}
int Cache::GetCacheAssociativityWays(){return cache_associativity_ways;}

Component::Component(int _id, string _name, int _componentType) : id(_id), name(_name), componentType(_componentType) { count = -1;}
Component::Component() :Component(0,"unknown",SYS_SAGE_COMPONENT_NONE){count = -1;}
Component::Component(Component * parent, int _id, string _name, int _componentType) : id(_id), name(_name), componentType(_componentType)
{
    SetParent(parent);
    parent->InsertChild(this);
}
Component::Component(Component * parent) :Component(0,"unknown",SYS_SAGE_COMPONENT_NONE)
{
    SetParent(parent);
    parent->InsertChild(this);
}


Topology::Topology():Component(0, "sys-sage Topology", SYS_SAGE_COMPONENT_TOPOLOGY){}

Memory::Memory():Component(0, "Memory", SYS_SAGE_COMPONENT_MEMORY){}
Memory::Memory(Component * parent, string _name, long long _size):Component(parent, 0, _name, SYS_SAGE_COMPONENT_MEMORY), size(_size){}
Memory::Memory(Component * parent, string _name):Memory(parent, _name, -1){}
Memory::Memory(Component * parent):Memory(parent, "Memory"){}

Storage::Storage():Component(0, "Storage", SYS_SAGE_COMPONENT_STORAGE){}
Storage::Storage(Component * parent):Component(parent, 0, "Storage", SYS_SAGE_COMPONENT_STORAGE){}

Node::Node(int _id):Component(_id, "Node", SYS_SAGE_COMPONENT_NODE){}
Node::Node():Node(0){}
Node::Node(Component * parent, int _id):Component(parent, _id, "Node", SYS_SAGE_COMPONENT_NODE){}
Node::Node(Component * parent):Node(parent, 0){}

Chip::Chip(int _id, string _name, int _type):Component(_id, _name, SYS_SAGE_COMPONENT_CHIP), type(_type) {}
Chip::Chip(int _id, string _name):Chip(_id, _name, SYS_SAGE_CHIP_TYPE_NONE){}
Chip::Chip(int _id):Chip(_id, "Chip"){}
Chip::Chip():Chip(0){}
Chip::Chip(Component * parent, int _id, string _name, int _type):Component(parent, _id, _name, SYS_SAGE_COMPONENT_CHIP), type(_type){}
Chip::Chip(Component * parent, int _id, string _name):Chip(parent, _id, _name, SYS_SAGE_CHIP_TYPE_NONE){}
Chip::Chip(Component * parent, int _id):Chip(parent, _id, "Chip"){}
Chip::Chip(Component * parent):Chip(parent, 0){}

Cache::Cache(int _id, int  _cache_level, unsigned long long _cache_size, int _associativity, int _cache_line_size): Component(_id, "Cache", SYS_SAGE_COMPONENT_CACHE), cache_type(to_string(_cache_level)), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
Cache::Cache(int _id, int  _cache_level, unsigned long long _cache_size, int _associativity): Cache(_id, _cache_level, _cache_size, _associativity, -1){}
Cache::Cache():Cache(0,0,-1,-1){}
Cache::Cache(Component * parent, int _id, string _cache_type, unsigned long long _cache_size, int _associativity, int _cache_line_size): Component(parent, _id, "Cache", SYS_SAGE_COMPONENT_CACHE), cache_type(_cache_type), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
Cache::Cache(Component * parent, int _id, int _cache_level, unsigned long long _cache_size, int _associativity, int _cache_line_size): Cache(parent, _id, to_string(_cache_level), _cache_size, _associativity, -1){}
Cache::Cache(Component * parent, int _id, int _cache_level, unsigned long long _cache_size, int _associativity): Cache(parent, _id, _cache_level, _cache_size, _associativity, -1){}
Cache::Cache(Component * parent, int _id, string _cache_type):Cache(parent, _id, _cache_type, 0, -1, -1){}
Cache::Cache(Component * parent):Cache(parent,0,0,-1,-1){}

Subdivision::Subdivision(Component * parent, int _id, string _name, int _componentType): Component(parent, _id, _name, _componentType){}
Subdivision::Subdivision(Component * parent, int _id, string _name): Subdivision(parent, _id, _name, SYS_SAGE_COMPONENT_SUBDIVISION){}
Subdivision::Subdivision(Component * parent, int _id, int _componentType): Subdivision(parent, _id, "Subdivision", _componentType){}
Subdivision::Subdivision(Component * parent, int _id): Subdivision(parent, _id, "Subdivision", SYS_SAGE_COMPONENT_SUBDIVISION){}
Subdivision::Subdivision(Component * parent):Subdivision(parent,0){}
Subdivision::Subdivision(int _id, string _name, int _componentType): Component(_id, _name, _componentType){}
Subdivision::Subdivision(int _id, int _componentType): Subdivision(_id, "Subdivision", _componentType){}
Subdivision::Subdivision(int _id, string _name): Subdivision(_id, _name, SYS_SAGE_COMPONENT_SUBDIVISION){}
Subdivision::Subdivision(int _id): Subdivision(_id, "Subdivision", SYS_SAGE_COMPONENT_SUBDIVISION){}
Subdivision::Subdivision():Subdivision(0){}

Numa::Numa(int _id, int _size):Subdivision(_id, "Numa", SYS_SAGE_COMPONENT_NUMA), size(_size){}
Numa::Numa(int _id):Numa(_id, 0){}
Numa::Numa():Numa(0){}
Numa::Numa(Component * parent, int _id, long long _size):Subdivision(parent, _id, "Numa", SYS_SAGE_COMPONENT_NUMA), size(_size){}
Numa::Numa(Component * parent, int _id):Numa(parent, _id, -1){}
Numa::Numa(Component * parent):Numa(parent, 0){}

Core::Core(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_CORE){}
Core::Core(int _id):Core(_id, "Core"){}
Core::Core():Core(0){}
Core::Core(Component * parent, int _id, string _name):Component(parent, _id, _name, SYS_SAGE_COMPONENT_CORE){}
Core::Core(Component * parent, int _id):Core(parent, _id, "Core"){}
Core::Core(Component * parent):Core(parent, 0){}

Thread::Thread(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_THREAD){}
Thread::Thread(int _id):Thread(_id, "Thread"){}
Thread::Thread():Thread(0){}
Thread::Thread(Component * parent, int _id, string _name):Component(parent, _id, _name, SYS_SAGE_COMPONENT_THREAD){}
Thread::Thread(Component * parent, int _id):Thread(parent, _id, "Thread"){}
Thread::Thread(Component * parent):Thread(parent, 0){}


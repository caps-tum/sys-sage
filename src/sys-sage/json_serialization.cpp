#include <sys-sage.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <utility>
#include <queue>

using namespace sys_sage;
using json = nlohmann::json;

template <typename T>
static void DumpAttributes(json &obj, T *compOrRel)
{
    json attributes;

    for (auto it = compOrRel->AttributesBegin(); it != compOrRel->AttributesEnd(); it++) {
        auto &key = it->first;
        auto &val = it->second;

        nlohmann::json attribute;
        val->Serialize(attribute);

        if (!attribute.is_null()) // only export the attribute if JSON expored is defined for this type
            attributes[key] = std::move(attribute);
    }

    if (attributes.size() > 0)
        obj["attributes"] = std::move(attributes);
}

template <typename T>
static void LoadAttributes(const json &obj, T *compOrRel)
{
    for (auto it = obj.begin(); it != obj.end(); it++) {
        auto &key = it.key();
        auto &value = it.value();

        // TODO: define a constant string literal instead of using "_sys_sage_type" and "_sys_sage_value" directly
        auto typeIt = value.find("_sys_sage_type");
        if (typeIt == value.end())
            continue;

        std::string type = typeIt->get<std::string>();
        auto callBack = sys_sage::TypeRegistry::Instance().GetCallBack(type);

        if (!callBack)
            continue;

        auto attribute = (*callBack)(value["_sys_sage_value"]);
        // TODO: somehow make this private
        compOrRel->_EmplaceAttribute(key, attribute);
    }
}

static Component *ComponentFromJson(const json &obj,
                                    std::unordered_map<uintptr_t, Component *> &componentMap)
{
    Component *component;

    from_json(obj, component);
    if (component == nullptr)
        return nullptr;

    auto address = obj["address"].get<uintptr_t>();
    componentMap[address] = component;

    return component;
}

static Relation *RelationFromJson(const json &obj,
                                  const std::unordered_map<uintptr_t, Component *> &componentMap)
{
    Relation *relation;

    std::string relationType = obj["type"].get<std::string>();
    if (relationType == "Relation") {
        relation = new Relation;
    } else if (relationType == "DataPath") {
        relation = new DataPath;
    } else if (relationType == "QuantumGate") {
        relation = new QuantumGate;
    } else if (relationType == "CouplingMap") {
        relation = new CouplingMap;
    } else {
        std::cerr << "error: invalid relation type '" << relationType << "'\n";
        return nullptr;
    }

    if (relation->_FromJson(obj, componentMap)) {
        delete relation;
        return nullptr;
    }

    return relation;
}

static void CollectComponentsInSubtree(const Component *root,
                                       std::unordered_set<const Component *> &componentsInSubtree)
{
    std::queue<const Component *> queue;
    queue.push(root);
    componentsInSubtree.insert(root);

    do {
        auto comp = queue.front();
        queue.pop();

        for (auto child : comp->GetChildren()) {
            queue.push(child);
            componentsInSubtree.insert(child);
        }
    } while (!queue.empty());
}

void sys_sage::DumpJson(const Component *component, json &obj)
{
    obj["componentTree"] = component;

    std::unordered_set<const Component *> componentsInSubtree;
    CollectComponentsInSubtree(component, componentsInSubtree);

    std::vector<json> relationSubgraph;

    for (auto comp : componentsInSubtree) {
        for (auto relationType : RelationType::RelationTypeList) {
            auto &relations = comp->GetRelationsByType(relationType);
            for (auto relation : relations) {
                // print each relation once
                if (relation->GetComponent(0) != comp)
                    continue;

                bool inSubtree = true;
                for (auto rComp : relation->GetComponents()) {
                    // only collect relations that connect components of the subtree
                    if (componentsInSubtree.count(rComp) == 0) {
                        inSubtree = false;
                        break;
                    }
                }
                if (inSubtree)
                  relationSubgraph.push_back(relation);
            }
        }
    }

    if (relationSubgraph.size() > 0)
        obj["relationGraph"] = std::move(relationSubgraph);
}

int sys_sage::DumpJson(const Component *component,
                       const std::filesystem::path &path)
{
    json obj;
    DumpJson(component, obj);

    if (path.empty()) {
        std::cout << obj.dump(2) << std::endl;
    } else {
        std::ofstream stream ( path );
        if (!stream) {
            std::cerr << "Failed to write to " << path << "\n";
            return 1;
        }
        stream << obj.dump(4) << std::endl;
    }

    return 0;
}

Component *sys_sage::LoadJson(const json &obj)
{
    std::unordered_map<uintptr_t, Component *> componentMap;

    auto root = ComponentFromJson(obj["componentTree"], componentMap);
    if (root == nullptr)
        return nullptr;

    if (auto it = obj.find("relationGraph"); it != obj.end()) {
        for (auto &relation : *it) {
            if (RelationFromJson(relation, componentMap)) {
                delete root;
                return nullptr;
            }
        }
    }

    return root;
}

Component *sys_sage::LoadJson(const std::filesystem::path &path)
{
    std::ifstream stream ( path );
    if (!stream)
        return nullptr;

    const json obj = json::parse(stream);
    stream.close();

    return LoadJson(obj);
}

void sys_sage::to_json(json &obj, const Component *component)
{
    component->_ToJson(obj);
}

void sys_sage::from_json(const json &obj, Component *&component)
{
    std::string componentType = obj["type"].get<std::string>();
    if (componentType == "GenericComponent") {
        component = new Component;
    } else if (componentType == "HW_Thread") {
        component = new Thread;
    } else if (componentType == "Core") {
        component = new Core;
    } else if (componentType == "Cache") {
        component = new Cache;
    } else if (componentType == "Subdivision") {
        component = new Subdivision;
    } else if (componentType == "NUMA") {
        component = new Numa;
    } else if (componentType == "Chip") {
        component = new Chip;
    } else if (componentType == "Memory") {
        component = new Memory;
    } else if (componentType == "Storage") {
        component = new Storage;
    } else if (componentType == "Node") {
        component = new Node;
    } else if (componentType == "QuantumBackend") {
        component = new QuantumBackend;
    } else if (componentType == "AtomSite") {
        component = new AtomSite;
    } else if (componentType == "Qubit") {
        component = new Qubit;
    } else if (componentType == "Topology") {
        component = new Topology;
    } else {
        std::cerr << "error: invalid component type '" << componentType << "'\n";
        component = nullptr;
        return;
    }

    if (component->_FromJson(obj)) {
        delete component;
        component = nullptr;
    }
}

void sys_sage::to_json(json &obj, const Relation *relation)
{
    relation->_ToJson(obj);
}

void sys_sage::Component::_ToJson(json &obj) const
{
    obj["type"] = GetComponentTypeStr();
    obj["id"] = id;
    obj["address"] = reinterpret_cast<uintptr_t>(this);

    DumpAttributes(obj, this);

    if (children.size() > 0) {
        std::vector<json> jsonChildren ( children.size() );

        for (size_t i = 0; i < jsonChildren.size(); i++)
            // this recursive call chain might be very memory heavy
            children[i]->_ToJson(jsonChildren[i]);

      obj["children"] = jsonChildren;
    }
}

int sys_sage::Component::_FromJson(const json &obj)
{
    obj["id"].get_to<int>(id);

    if (auto it = obj.find("attributes"); it != obj.end())
        LoadAttributes(*it, this);

    if (auto it = obj.find("children"); it != obj.end()) {
        for (auto &childJson : *it) {
            auto child = childJson.get<Component *>();
            if (child == nullptr)
                return 1;

            child->SetParent(this);
            InsertChild(child);
        }
    }

    return 0;
}

void sys_sage::Relation::_ToJson(json &obj) const
{
    obj["type"] = GetTypeStr();
    obj["category"] = category;
    obj["id"] = id;
    obj["ordered"] = ordered;

    // use the memory addresses for unique identification
    std::vector<uintptr_t> addresses (components.size());

    for (size_t i = 0; i < components.size(); i++)
        addresses[i] = reinterpret_cast<uintptr_t>(components[i]);

    obj["components"] = addresses;

    DumpAttributes(obj, this);
}

int sys_sage::Relation::_FromJson(const json &obj,
                                  const std::unordered_map<uintptr_t, Component *> &componentMap)
{
    components.clear();

    for (auto &component : obj["components"]) {
        auto address = component["address"].get<uintptr_t>();

        auto componentIt = componentMap.find(address);
        if (componentIt == componentMap.end()) {
            std::cerr << "error: could not find component given by address '0x" << std::hex << address << "' when parsing the JSON\n";
            return 1;
        }
        components.push_back(componentIt->second);
    }

    obj["category"].get_to<RelationCategory::type>(category);
    obj["id"].get_to<int>(id);
    obj["ordered"].get_to<bool>(ordered);

    if (auto it = obj.find("attributes"); it != obj.end())
        LoadAttributes(*it, this);

    return 0;
}

void sys_sage::AtomSite::_ToJson(json &obj) const
{
    obj["SiteProperties"] = {
        { "nRows", properties.nRows },
        { "nColumns", properties.nColumns },
        { "nAods", properties.nAods },
        { "nAodIntermediateLevels", properties.nAodIntermediateLevels },
        { "nAodCoordinates", properties.nAodCoordinates },
        { "interQubitDistance", properties.interQubitDistance },
        { "interactionRadius", properties.interactionRadius },
        { "blockingFactor", properties.blockingFactor }
    };

    QuantumBackend::_ToJson(obj);
}

int sys_sage::AtomSite::_FromJson(const json &obj)
{
    obj["SiteProperties"]["nRows"].get_to<int>(properties.nRows);
    obj["SiteProperties"]["nColumns"].get_to<int>(properties.nColumns);
    obj["SiteProperties"]["nAods"].get_to<int>(properties.nAods);
    obj["SiteProperties"]["nAodIntermediateLevels"].get_to<int>(properties.nAodIntermediateLevels);
    obj["SiteProperties"]["nAodCoordinates"].get_to<int>(properties.nAodCoordinates);
    obj["SiteProperties"]["interQubitDistance"].get_to<double>(properties.interQubitDistance);
    obj["SiteProperties"]["interactionRadius"].get_to<double>(properties.interactionRadius);
    obj["SiteProperties"]["blockingFactor"].get_to<double>(properties.blockingFactor);

    int rval = QuantumBackend::_FromJson(obj);
    if (rval)
        return 1;


    return 0;
}

void sys_sage::Cache::_ToJson(json &obj) const
{
    obj["cacheType"] = cache_type;
    if (cache_size > 0)
        obj["cacheSize"] = cache_size;
    if (cache_associativity_ways > 0)
        obj["cacheAssociativityWays"] = cache_associativity_ways;
    if (cache_line_size > 0)
        obj["cacheLineSize"] = cache_line_size;

    Component::_ToJson(obj);
}

int sys_sage::Cache::_FromJson(const json &obj)
{
    obj["cacheType"].get_to<std::string>(cache_type);
    if (auto it = obj.find("cacheSize"); it != obj.end())
        it->get_to<long long>(cache_size);
    if (auto it = obj.find("cacheAssociativityWays"); it != obj.end())
        it->get_to<int>(cache_associativity_ways);
    if (auto it = obj.find("cacheLineSize"); it != obj.end())
        it->get_to<int>(cache_line_size);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Chip::_ToJson(json &obj) const
{
    obj["chipCategory"] = category;
    if (!vendor.empty())
        obj["vendor"] = vendor;
    if (!model.empty())
        obj["model"] = model;

    Component::_ToJson(obj);
}

int sys_sage::Chip::_FromJson(const json &obj)
{
    obj["chipCategory"].get_to<ChipCategory::type>(category);
    if (auto it = obj.find("vendor"); it != obj.end())
        it->get_to<std::string>(vendor);
    if (auto it = obj.find("model"); it != obj.end())
        it->get_to<std::string>(model);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Core::_ToJson(json &obj) const
{
#ifdef PROC_CPUINFO
    obj["frequency"] = freq;
#endif

    Component::_ToJson(obj);
}

int sys_sage::Core::_FromJson(const json &obj)
{
#ifdef PROC_CPUINFO
    if (auto it = obj.find("frequency"); it != obj.end())
      it->get_to<double>(freq);
#endif

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::CouplingMap::_ToJson(json &obj) const
{
    obj["fidelity"] = fidelity;

    Relation::_ToJson(obj);
}

int sys_sage::CouplingMap::_FromJson(const json &obj,
                                     const std::unordered_map<uintptr_t, Component *> &componentMap)
{
    obj["fidelity"].get_to<double>(fidelity);

    int rval = Relation::_FromJson(obj, componentMap);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::DataPath::_ToJson(json &obj) const
{
    obj["dataPathCategory"] = dp_category;
    obj["bandwidth"] = bw;
    obj["latency"] = latency;

    Relation::_ToJson(obj);
}

int sys_sage::DataPath::_FromJson(const json &obj,
                                  const std::unordered_map<uintptr_t, Component *> &componentMap)
{
    obj["dataPathCategory"].get_to<DataPathCategory::type>(dp_category);
    obj["bandwidth"].get_to<double>(bw);
    obj["latency"].get_to<double>(latency);

    int rval = Relation::_FromJson(obj, componentMap);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Memory::_ToJson(json &obj) const
{
    obj["volatile"] = is_volatile;
    if (size > 0)
        obj["size"] = size;

    Component::_ToJson(obj);
}

int sys_sage::Memory::_FromJson(const json &obj)
{
    obj["volatile"].get_to<bool>(is_volatile);

    if (auto it = obj.find("size"); it != obj.end())
        it->get_to<long long>(size);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;
    
    return 0;
}

void sys_sage::Node::_ToJson(json &obj) const
{
    Component::_ToJson(obj);
}

int sys_sage::Node::_FromJson(const json &obj)
{
    return Component::_FromJson(obj);
}

void sys_sage::Numa::_ToJson(json &obj) const
{
    if (size > 0)
        obj["size"] = size;

    Subdivision::_ToJson(obj);
}

int sys_sage::Numa::_FromJson(const json &obj)
{
    if (auto it = obj.find("size"); it != obj.end())
        it->get_to<long long>(size);

    int rval = Subdivision::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::QuantumBackend::_ToJson(json &obj) const
{
    obj["numQubits"] = num_qubits;

    Component::_ToJson(obj);
}

int sys_sage::QuantumBackend::_FromJson(const json &obj)
{
    obj["numQubits"].get_to<int>(num_qubits);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::QuantumGate::_ToJson(json &obj) const
{
    obj["gateSize"] = gate_size;
    obj["name"] = name;
    obj["gateLength"] = gate_length;
    obj["gateCategory"] = gate_category;
    obj["fidelity"] = fidelity;
    obj["unitary"] = unitary;

    Relation::_ToJson(obj);
}

int sys_sage::QuantumGate::_FromJson(const json &obj,
                                     const std::unordered_map<uintptr_t, Component *> &componentMap)
{
    obj["gateSize"].get_to<size_t>(gate_size);
    obj["name"].get_to<std::string>(name);
    obj["gateLength"].get_to<int>(gate_length);
    obj["gateCategory"].get_to<QuantumGateCategory::type>(gate_category);
    obj["fidelity"].get_to<double>(fidelity);
    obj["unitary"].get_to<std::string>(unitary);

    int rval = Relation::_FromJson(obj, componentMap);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Qubit::_ToJson(json &obj) const
{
    obj["q1Fidelity"] = q1_fidelity;
    obj["t1"] = t1;
    obj["t2"] = t2;
    obj["readoutFidelity"] = readout_fidelity;
    obj["readoutLength"] = readout_length;

    Component::_ToJson(obj);
}

int sys_sage::Qubit::_FromJson(const json &obj)
{
    obj["q1Fidelity"].get_to<double>(q1_fidelity);
    obj["t1"].get_to<double>(t1);
    obj["t2"].get_to<double>(t2);
    obj["readoutFidelity"].get_to<double>(readout_fidelity);
    obj["readoutLength"].get_to<double>(readout_length);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Storage::_ToJson(json &obj) const
{
    if (size > 0)
        obj["size"] = size;

    Component::_ToJson(obj);
}

int sys_sage::Storage::_FromJson(const json &obj)
{
    if (auto it = obj.find("size"); it != obj.end())
        it->get_to<long long>(size);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Subdivision::_ToJson(json &obj) const
{
    obj["subdivisionCategory"] = category;

    Component::_ToJson(obj);
}

int sys_sage::Subdivision::_FromJson(const json &obj)
{
    obj["subdivisionCategory"].get_to<SubdivisionCategory::type>(category);

    int rval = Component::_FromJson(obj);
    if (rval)
        return 1;

    return 0;
}

void sys_sage::Thread::_ToJson(json &obj) const
{
    Component::_ToJson(obj);
}

int sys_sage::Thread::_FromJson(const json &obj)
{
    return Component::_FromJson(obj);
}

void sys_sage::Topology::_ToJson(json &obj) const
{
    Component::_ToJson(obj);
}

int sys_sage::Topology::_FromJson(const json &obj)
{
    return Component::_FromJson(obj);
}

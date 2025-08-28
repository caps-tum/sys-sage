#ifndef SYS_SAGE_BINDINGS_CPP
#define SYS_SAGE_BINDINGS_CPP

#include "defines.hpp"
#ifdef PY_SYS_SAGE

#include <cstdio>
#include <exception>
#include <libxml2/libxml/parser.h>
#include <optional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/attr.h>
#include <string>
#include <tuple>

#include "sys-sage.hpp"

namespace py = pybind11;

std::vector<std::string> default_attribs = {"CATcos","CATL3mask","mig_size","Number_of_streaming_multiprocessors","Number_of_cores_in_GPU","Number_of_cores_per_SM","Bus_Width_bit","Clock_Frequency","latency","latency_min","latency_max","CUDA_compute_capability","mig_uuid","freq_history","GPU_Clock_Rate"};

py::function print_attributes;
py::function print_complex_attributes;

py::function read_attributes;
py::function read_complex_attributes;

int xmldumper(std::string key, void* value, std::string* ret_value_str) {
    //
    if(default_attribs.end() != std::find(default_attribs.begin(), default_attribs.end(), key))
        return 0;
    auto * ptr = static_cast<std::shared_ptr<py::object>*>(value);
    py::object res = print_attributes( py::cast(key), *ptr->get());
    //check if res is none
    if(res.is_none())
        return 0;
    *ret_value_str = res.cast<std::string>();
    return 1;
}

int xmldumper_complex(std::string key, void* value, xmlNodePtr node) {
    if(default_attribs.end() != std::find(default_attribs.begin(), default_attribs.end(), key))
        return 0;
    auto * ptr = static_cast<std::shared_ptr<py::object>*>(value);
    //idea: we expect fcn to return xml as a string and write it to node
    py::object res = print_complex_attributes(py::cast(key), *ptr->get());
    //check if res is none
    if(res.is_none())
        return 0;
    //xmlBufferPtr buffer = xmlBufferCreate();
    std::string xml_str = res.cast<std::string>();
    xmlDocPtr doc = xmlParseDoc(BAD_CAST xml_str.c_str());
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlAddChild(node,root->children);
    return 1;
}

void* xmlloader(xmlNodePtr node) {
    //idea: we expect fcn to return xml as a string and write it to node
    xmlBufferPtr buffer = xmlBufferCreate();
    try{
        xmlNodeDump(buffer, node->doc, node, 0, 1);
        std::string xml_str(reinterpret_cast<const char*>(xmlBufferContent(buffer)));
        xmlBufferFree(buffer);
        py::object value = read_attributes(py::cast(xml_str));
        //check for values content
        if (value.is_none()) {
            return NULL;
        }
        std::shared_ptr<py::object> *ptr = new std::shared_ptr<py::object>(std::make_shared<py::object>(value));
        return static_cast<void*>(ptr);
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
        return NULL;
    }
}

int xmlloader_complex(xmlNodePtr node, sys_sage::Component *c) {
    xmlBufferPtr buffer = xmlBufferCreate();
    try{
        xmlNodeDump(buffer, node->doc, node, 0, 1);
        std::string xml_str(reinterpret_cast<const char*>(xmlBufferContent(buffer)));
        xmlBufferFree(buffer);
        py::object comp = py::cast(c);
        py::object value = read_complex_attributes(py::cast(xml_str),comp);
        if (value.is_none())
            return 0;
        return 1;
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
    }
    return 0;
}

//TODO: Add dynamic allocation for values
//TODO: Delete values only after success
template <typename T>
void set_attribute(T &self, const std::string &key, py::object &value) {
    //std::cout << "set attribute: " << key << " = " << value << std::endl;

    auto val = self.attrib.find(key);
    
    if (!key.compare("CATcos") || !key.compare("CATL3mask")) {
        void * newval = static_cast<void*>(new uint64_t(py::cast<uint64_t>(value)));
        if(val != self.attrib.end())
            delete static_cast<uint64_t*>(val->second);
        self.attrib[key] = newval;
    } else if (!key.compare("mig_size")) {
        void* new_val = static_cast<void*>(new long long(py::cast<long long>(value)));
        if(val != self.attrib.end())
            delete static_cast<long long*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("Number_of_streaming_multiprocessors") || 
               !key.compare("Number_of_cores_in_GPU") || 
               !key.compare("Number_of_cores_per_SM") || 
               !key.compare("Bus_Width_bit")) {
        void* new_val = static_cast<void*>(new int(py::cast<int>(value)));
        if(val != self.attrib.end())
            delete static_cast<int*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("Clock_Frequency") || !key.compare("GPU_Clock_Rate")) {
        void * new_val = static_cast<void*>(new double(py::cast<double>(value)));
        if(val != self.attrib.end())
            delete static_cast<double*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("latency") ||
               !key.compare("latency_min") ||
               !key.compare("latency_max")) {
        void * new_val = static_cast<void*>(new float(py::cast<float>(value)));
        if(val != self.attrib.end())
            delete static_cast<float*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("CUDA_compute_capability") || 
               !key.compare("mig_uuid")) {
        void * new_val= static_cast<void*>(new std::string(py::cast<std::string>(value)));
        if(val != self.attrib.end())
            delete static_cast<std::string*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("freq_history")) {
        auto fh = new std::vector<std::tuple<long long, double>>;
        py::dict fh_dict = py::cast<py::dict>(value);
        for (auto [key, value] : fh_dict) {
            fh->push_back(std::make_tuple(py::cast<long long>(key), py::cast<double>(value)));
        }
        void * new_val = static_cast<void*>(fh);
        if(val != self.attrib.end())
            delete static_cast<std::vector<std::tuple<long long, double>>*>(val->second);
        self.attrib[key] = new_val;
    //} else if (!key.compare("GPU_Clock_Rate")) {
    //    std::cout << "Setting attribute: " << key << std::endl;
    //    py::object freq = value["freq"];
    //    py::str unit = value["unit"];
    //    //double * f = new double(py::cast<double>(freq));
    //    //std::string * u = new std::string(py::cast<std::string>(unit));
    //    void * new_val = static_cast<void*>(new std::tuple<double, std::string>(py::cast<double>(freq), py::cast<std::string>(unit)));
    //    if(val != self.attrib.end())
    //        delete static_cast<std::tuple<double, std::string>*>(val->second);
    //    self.attrib[key] = new_val;
    } else {
        void * new_val = static_cast<void*>(new std::shared_ptr<py::object>(
            std::make_shared<py::object>(value)));
        if (val != self.attrib.end())
            delete static_cast<std::shared_ptr<py::object>*>(val->second);
        self.attrib[key] = new_val;
    }

}

template <typename T>
py::object get_attribute(T &self, const std::string &key) {
    auto val = self.attrib.find(key);
    if (val != self.attrib.end()) {
        if(!key.compare("CATcos") || !key.compare("CATL3mask")){
            uint64_t retval = *(reinterpret_cast<uint64_t*>(val->second)); 
            return py::cast(retval);
        }
        else if(!key.compare("mig_size") )
        {
            return py::cast(*reinterpret_cast<long long*>(val->second));
        }
        //val->secondue: int
        else if(!key.compare("Number_of_streaming_multiprocessors") || 
        !key.compare("Number_of_cores_in_GPU") || 
        !key.compare("Number_of_cores_per_SM")  || 
        !key.compare("Bus_Width_bit") )
        {
            return py::cast(*reinterpret_cast<int*>(val->second));
        }
        //value: double
        else if(!key.compare("Clock_Frequency") || !key.compare("GPU_Clock_Rate"))
        {
            return py::cast(*reinterpret_cast<double*>(val->second));
        }
        //value: float
        else if(!key.compare("latency") ||
        !key.compare("latency_min") ||
        !key.compare("latency_max") )
        {
            return py::cast(*reinterpret_cast<float*>(val->second));
        }   
        //value: string
        else if(!key.compare("CUDA_compute_capability") || 
        !key.compare("mig_uuid") )
        {
            return py::cast(*reinterpret_cast<std::string*>(val->second));
        }
        else if(!key.compare("freq_history") ){
            std::vector<std::tuple<long long,double>>* value = reinterpret_cast<std::vector<std::tuple<long long,double>>*>(val->second);
            py::dict freq_dict;
             for(auto [ ts,freq ] : *value){
                 freq_dict[py::cast(ts)] = py::cast(freq);
                 //printf("ts:%lld freq:%f\n",ts,freq);
             }
             return freq_dict;
        //}
        //else if(!key.compare("GPU_Clock_Rate")){
        //    auto value = static_cast<std::tuple<double, std::string>*>(val->second);
        //    auto [ freq, unit ] = *value;
        //    py::dict freq_dict;
        //    freq_dict[py::str("freq")] = py::cast(freq);
        //    freq_dict[py::str("unit")] = py::cast(unit);  
        //    return freq_dict;     
        }else{
            auto * ptr = static_cast<std::shared_ptr<py::object>*>(val->second);
            return *ptr->get();
        }
    } else {
        throw py::attribute_error("Attribute '" + key + "' not found"); 
    }
}

template <typename T>
py::object get_attribute(T &self, int pos){
    if (pos < 0 || static_cast<size_t>(pos) >= self.attrib.size())
        throw py::index_error("Index out of bounds");
    auto it = self.attrib.begin();
    std::advance(it, pos);
    return get_attribute(self, it->first);
}


template <typename T>
void remove_attribute(T &self, const std::string &key) {
    auto val = self.attrib.find(key);
    if (val != self.attrib.end()) {
        delete static_cast<std::shared_ptr<py::object>*>(val->second);
        self.attrib.erase(val);
    } else {
        throw py::attribute_error("Attribute " + key + " not found");
    }
}


PYBIND11_MODULE(sys_sage, m) {
    using namespace sys_sage;

    m.attr("COMPONENT_NONE") = ComponentType::None;
    m.attr("COMPONENT_THREAD") = ComponentType::Thread;
    m.attr("COMPONENT_CORE") = ComponentType::Core;
    m.attr("COMPONENT_CACHE") = ComponentType::Cache;
    m.attr("COMPONENT_SUBDIVISION") = ComponentType::Subdivision;
    m.attr("COMPONENT_NUMA") = ComponentType::Numa;
    m.attr("COMPONENT_CHIP") = ComponentType::Chip;
    m.attr("COMPONENT_MEMORY") = ComponentType::Memory;
    m.attr("COMPONENT_STORAGE") = ComponentType::Storage;
    m.attr("COMPONENT_NODE") = ComponentType::Node;
    m.attr("COMPONENT_QUANTUMBACKEND") = ComponentType::QuantumBackend;
    m.attr("COMPONENT_ATOMSITE") = ComponentType::AtomSite;
    m.attr("COMPONENT_QUBIT") = ComponentType::Qubit;
    m.attr("COMPONENT_TOPOLOGY") = ComponentType::Topology;

    m.attr("SUBDIVISION_TYPE_NONE") = SubdivisionType::None;
    m.attr("SUBDIVISION_TYPE_GPU_SM") = SubdivisionType::GpuSM;

    m.attr("CHIP_TYPE_NONE") = ChipType::None;
    m.attr("CHIP_TYPE_CPU") = ChipType::Cpu;
    m.attr("CHIP_TYPE_CPU_SOCKET") = ChipType::CpuSocket;
    m.attr("CHIP_TYPE_GPU") = ChipType::Gpu;

    m.attr("RELATION_TYPE_ANY") = RelationType::Any;
    m.attr("RELATION_TYPE_RELATION") = RelationType::Relation;
    m.attr("RELATION_TYPE_DATAPATH") = RelationType::DataPath;
    m.attr("RELATION_TYPE_QUANTUMGATE") = RelationType::QuantumGate;
    m.attr("RELATION_TYPE_COUPLINGMAP") = RelationType::CouplingMap;

    m.attr("DATAPATH_TYPE_ANY") = DataPathType::Any;
    m.attr("DATAPATH_TYPE_NONE") = DataPathType::None;
    m.attr("DATAPATH_TYPE_LOGICAL") = DataPathType::Logical;
    m.attr("DATAPATH_TYPE_PHYSICAL") = DataPathType::Physical;
    m.attr("DATAPATH_TYPE_DATATRANSFER") = DataPathType::Datatransfer;
    m.attr("DATAPATH_TYPE_L3CAT") = DataPathType::L3CAT;
    m.attr("DATAPATH_TYPE_MIG") = DataPathType::MIG;
    m.attr("DATAPATH_TYPE_C2C") = DataPathType::C2C;

    m.attr("DATAPATH_DIRECTION_ANY") = DataPathDirection::Any;
    m.attr("DATAPATH_DIRECTION_OUTGOING") = DataPathDirection::Outgoing;
    m.attr("DATAPATH_DIRECTION_INCOMING") = DataPathDirection::Incoming;

    m.attr("DATAPATH_ORIENTATION_ORIENTED") = DataPathOrientation::Oriented;
    m.attr("DATAPATH_ORIENTATION_BIDIRECTIONAL") = DataPathOrientation::Bidirectional;

    m.attr("QUANTUMGATE_TYPE_UNKNOWN") = QuantumGateType::Unknown;
    m.attr("QUANTUMGATE_TYPE_ID") = QuantumGateType::Id;
    m.attr("QUANTUMGATE_TYPE_X") = QuantumGateType::X;
    m.attr("QUANTUMGATE_TYPE_RZ") = QuantumGateType::Rz;
    m.attr("QUANTUMGATE_TYPE_CNOT") = QuantumGateType::Cnot;
    m.attr("QUANTUMGATE_TYPE_SX") = QuantumGateType::Sx;
    m.attr("QUANTUMGATE_TYPE_TOFFOLI") = QuantumGateType::Toffoli;

    //bind component class
    py::class_<Component, std::unique_ptr<Component, py::nodelete>>(m, "Component")
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name") = "unknown")
        .def(py::init<Component *, int, std::string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "unknown")
        .def("__setitem__", [](Component& self, const std::string& name, py::object value) {
            set_attribute<Component>(self,name, value);
        })
        .def("__getitem__", [](Component& self, const std::string& name) {
            return get_attribute<Component>(self,name);
        })
        .def("__getitem__", [](Component& self, int pos) {
            return get_attribute<Component>(self,pos);
        })
        .def("__delitem__", [](Component& self, const std::string& name) {
            remove_attribute<Component>(self,name);
        })
        .def("InsertChild", &Component::InsertChild, py::arg("child"), "Insert a child component")
        .def("InsertBetweenParentAndChild", &Component::InsertBetweenParentAndChild, py::arg("parent"), py::arg("child"), py::arg("alreadyParentsChild"),"Insert a component between parent and child")
        .def("InsertBetweenParentAndChildren", &Component::InsertBetweenParentAndChildren, py::arg("parent"), py::arg("children"), py::arg("alreadyParentsChildren"), "Insert a component between parent and children")
        .def("RemoveChild", &Component::RemoveChild, py::arg("child"),"Remove a child component")
        .def_property("parent", &Component::GetParent, &Component::SetParent, "The parent of the component")
        .def("SetParent", &Component::SetParent, py::arg("parent"), "Set the parent of the component")
        .def("PrintSubtree", &Component::PrintSubtree, "Print the subtree of the component up to level 0")
        .def("PrintAllRelationsInSubtree", &Component::PrintAllRelationsInSubtree, py::arg("relationType") = RelationType::Any, "Print all relations in the subtree")
        .def_property("name", &Component::GetName, &Component::SetName, "The name of the component")
        .def_property_readonly("id", &Component::GetId, "The id of the component")
        .def_property_readonly("type", &Component::GetComponentType, "The type of the component")
        .def("GetComponentTypeStr", &Component::GetComponentTypeStr, "The type of the component as string")
        .def("GetChildren", &Component::GetChildren, "The children of the component")
        .def("GetChild", &Component::GetChild, py::arg("id"), "Like get_child_by_id()")
        .def("GetChildById", &Component::GetChildById, py::arg("id"), "Get the first child component by id")
        .def("GetChildByType", &Component::GetChildByType, py::arg("type"), "Get the first child component by type")
        //vector<Component*> as input doesnt work
        .def("GetAllChildrenByType", (std::vector<Component*> (Component::*)(ComponentType::type) const)(&Component::GetAllChildrenByType), py::arg("type"), "Get all child components by type")
        .def("GetAllSubcomponentsByType", (std::vector<Component*> (Component::*)(ComponentType::type))(&Component::GetAllSubcomponentsByType),py::arg("type") ,"Get all sub components by type")
        .def("CountAllSubcomponents", &Component::CountAllSubcomponents, "Count all sub components")
        .def("CountAllSubcomponentsByType", &Component::CountAllSubcomponentsByType, py::arg("type"),"Count sub components by type")
        .def("CountChildrenByType", &Component::CountAllChildrenByType,py::arg("type"),"Count children by type")
        .def("GetAncestorByType", &Component::GetAncestorByType, py::arg("type"),"Get the first ancestor component by type")
        .def("GetSubtreeDepth", &Component::GetSubtreeDepth, "Get the depth of the subtree")
        .def("GetNthAncestor", &Component::GetNthAncestor, py::arg("n"),"Get the nth ancestor of the component")
        .def("GetNthDescendents", (std::vector<Component*> (Component::*)(int))&Component::GetNthDescendents,py::arg("n"),"Get all the nth descendents of the component")
        .def("GetSubcomponentsByType", (std::vector<Component*> (Component::*)(ComponentType::type))&Component::GetSubcomponentsByType,py::arg("type"),"Get all the sub components of the component by type")
        .def("GetComponentsInSubtree", (std::vector<Component*> (Component::*)())&Component::GetComponentsInSubtree,"Get all the components in the subtree of the component")
        .def("GetSubcomponentById", &Component::GetSubcomponentById, py::arg("id"),py::arg("type"),"Get the first sub component by id and type")
        .def("GetRelations", &Component::GetRelations, py::arg("type"), "Get all relations of that type")
        .def("GetAllRelationsBy", &Component::GetAllRelationsBy, py::arg("type") = RelationType::Any, py::arg("position") = -1, "Get all relations of that type and position")
        .def("GetDataPathByType", &Component::GetDataPathByType, py::arg("type"), py::arg("direction") = DataPathDirection::Any,"Get the first data path associated with the component by type")
        .def("GetAllDataPaths", (std::vector<DataPath *> (Component::*)(DataPathType::type, DataPathDirection::type) const)&Component::GetAllDataPaths, py::arg("type") = DataPathType::Any, py::arg("direction") = DataPathDirection::Any, "Get all datapaths of that type and direction")
        .def("CheckComponentTreeConsistency", &Component::CheckComponentTreeConsistency,"Check if the component tree is consistent")
        // pybind11 doesn't support pass-by-reference or pass-by-pointer of primitive types.
        // -> use a tuple instead of output parameters
        .def("GetTopologySize", [] (Component &self) {
            unsigned out_component_size = 0;
            unsigned out_dataPathSize = 0;
            int total_bytes = self.GetTopologySize(&out_component_size, &out_dataPathSize);
            return std::make_tuple(total_bytes, out_component_size, out_dataPathSize);
        }, "Get the size of the topology")
        .def("GetDepth", &Component::GetDepth,py::arg("refresh"),"Get the depth of the component, if refresh is true it will update the depth")
        .def("DeleteRelation", &Component::DeleteRelation, py::arg("relation"), "Delete the given relation from the component")
        .def("DeleteAllRelations", &Component::DeleteAllRelations, py::arg("type") = RelationType::Any,"Delete all relations of that type from the component")
        .def("DeleteSubtree", &Component::DeleteSubtree,"Delete the subtree of the component")
        .def("Delete", &Component::Delete,py::arg("withSubtree") = true,"Delete the component")
        .def("__bool__",[](Component& self){
            std::vector<Component*> children = self.GetChildren();
            return !children.empty();
        })
        .def("__repr__", [](Component& self) {
            //TODO: add more info
            return "<Component: " + self.GetName() + ">";
            });

    py::class_<Topology, std::unique_ptr<Topology, py::nodelete>,Component>(m, "Topology")
        .def(py::init<>());

    py::class_<Node, std::unique_ptr<Node, py::nodelete>, Component>(m, "Node")
        #ifdef INTEL_PQOS
        .def("UpdateL3CATCoreCOS", &Node::UpdateL3CATCoreCOS, "Create new data paths between all cores of the node and the L3 cache to reflect new L3 cache settings")
        #endif
        #ifdef PROC_CPUINFO
        .def("RefreshCpuCoreFrequency", &Node::RefreshCpuCoreFrequency, py::arg("keep_history")=false,"Refresh the cpu core frequency")
        #endif
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name")= "Node")
        .def(py::init<Component*, int, std::string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Node");

    py::class_<Memory,std::unique_ptr<Memory, py::nodelete>, Component>(m, "Memory")
        #ifdef NVIDIA_MIG
        .def("GetMIGSize", &Memory::GetMIGSize, py::arg("uuid"), "Get the MIG size of the memory element")
        #endif
        .def(py::init<long long, bool>(), py::arg("size") = -1, py::arg("isVolatile") = false)
        .def(py::init<Component*,int, std::string, long long, bool>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Memory", py::arg("size")=-1, py::arg("isVolatile")=false)
        .def_property("size", &Memory::GetSize, &Memory::SetSize, "The size of the memory")
        .def_property("isVolatile", &Memory::GetIsVolatile, &Memory::SetIsVolatile, "Whether the memory is volatile or not");

    py::class_<Storage, std::unique_ptr<Storage, py::nodelete>, Component>(m, "Storage")
        .def(py::init<long long>(), py::arg("size")=-1)
        .def(py::init<Component*,long long>(), py::arg("parent"), py::arg("size")= -1)
        .def_property("size", &Storage::GetSize, &Storage::SetSize, "The size of the storage");

    py::class_<Chip,std::unique_ptr<Chip, py::nodelete>, Component>(m, "Chip")
        #ifdef NVIDIA_MIG
        .def("GetMIGNumCores", &Chip::GetMIGNumCores, py::arg("uuid"))
        .def("GetMIGNumSMs", &Chip::GetMIGNumSMs, py::arg("uuid"))
        .def("UpdateMIGSettings", &Chip::UpdateMIGSettings, py::arg("uuid"))
        #endif
        .def(py::init<int,std::string,ChipType::type,std::string,std::string>(), py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType")= ChipType::None, py::arg("vendor") = "", py::arg("model") = "")
        .def(py::init<Component*,int,std::string,ChipType::type,std::string,std::string>(), py::arg("parent"),py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType") = ChipType::None, py::arg("vendor") = "", py::arg("model") = "")
        .def_property("vendor", &Chip::GetVendor, &Chip::SetVendor, "The vendor of the chip")
        .def_property("model", &Chip::GetModel, &Chip::SetModel, "The model of the chip")
        .def_property("chipType", &Chip::GetChipType, &Chip::SetChipType, "The type of the chip");

    py::class_<Cache, std::unique_ptr<Cache, py::nodelete>, Component>(m, "Cache")
        #ifdef NVIDIA_MIG
        .def("GetMIGSize", &Cache::GetMIGSize, py::arg("uuid"))
        #endif
        .def(py::init<int,int,long long, int, int>(), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*,int,int,long long, int, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*, int, std::string, long long, int, int>(), py::arg("parent"), py::arg("id"), py::arg("cache_type"), py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def_property("cacheLevel", &Cache::GetCacheLevel, &Cache::SetCacheLevel, "The level of the cache")
        .def_property("cacheName", &Cache::GetCacheName, &Cache::SetCacheName, "The name of the cache")
        .def_property("cacheSize", &Cache::GetCacheSize, &Cache::SetCacheSize, "The size of the cache")
        .def_property("cacheAssociativity", &Cache::GetCacheAssociativityWays, &Cache::SetCacheAssociativityWays, "The associativity of the cache")
        .def_property("cacheLineSize", &Cache::GetCacheLineSize, &Cache::SetCacheLineSize, "The line size of the cache");

    py::class_<Subdivision, std::unique_ptr<Subdivision, py::nodelete>, Component>(m, "Subdivision")
        .def(py::init<int,std::string>(), py::arg("id") = 0, py::arg("name") = "Subdivision")
        .def(py::init<Component*,int,std::string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Subdivision")
        .def_property("subdivisionType", &Subdivision::GetSubdivisionType, &Subdivision::SetSubdivisionType, "The type of the subdivision");

    py::class_<Numa,std::unique_ptr<Numa, py::nodelete>, Subdivision>(m, "Numa")
        .def(py::init<int, long long>(), py::arg("id") = 0, py::arg("size") = -1)
        .def(py::init<Component*, int, long long>(), py::arg("parent"), py::arg("id") = 0, py::arg("size") = -1)
        .def_property("size", &Numa::GetSize, &Numa::SetSize, "Size of the NUMA region");

    py::class_<Core, std::unique_ptr<Core, py::nodelete>, Component>(m, "Core")
        #ifdef PROC_CPUINFO
        .def("RefreshFreq", &Core::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property("freq", &Core::GetFreq, &Core::SetFreq, "Frequency of this core")
        #endif
        .def(py::init<int,std::string>(),py::arg("id") = 0, py::arg("name") = "Core")
        .def(py::init<Component*,int,std::string>(),py::arg("parent"),py::arg("id") = 0 ,py::arg("name") = "Core");

    py::class_<Thread, std::unique_ptr<Thread, py::nodelete>,Component>(m,"Thread")
        #ifdef INTEL_PQOS
        .def("GetCATAwareL3Size", &Thread::GetCATAwareL3Size, "Get L3 size of this thread")
        #endif
        #ifdef PROC_CPUINFO
        .def("RefreshFreq", &Thread::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property_readonly("freq", &Thread::GetFreq, "Get Frequency of this thread")
        #endif
        .def(py::init<int,std::string>(),py::arg("id") = 0,py::arg("name") = "Thread")
        .def(py::init<Component*,int,std::string>(),py::arg("parent"),py::arg("id") = 0,py::arg("name") = "Thread");

    py::class_<Qubit, std::unique_ptr<Qubit, py::nodelete>, Component>(m, "Qubit")
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name") = "Qubit")
        .def(py::init<Component *, int, std::string> (), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Qubit")
        .def_property_readonly("t1", &Qubit::GetT1)
        .def_property_readonly("t2", &Qubit::GetT2)
        .def_property_readonly("readout_fidelity", &Qubit::GetReadoutFidelity)
        .def_property_readonly("q1_fidelity", &Qubit::Get1QFidelity)
        .def_property_readonly("readout_length", &Qubit::GetReadoutLength)
        .def_property_readonly("frequency", &Qubit::GetFrequency)
        .def_property_readonly("calibration_time", &Qubit::GetCalibrationTime)
        .def("SetProperties", &Qubit::SetProperties, py::arg("t1"), py::arg("t2"), py::arg("readout_fidelity"), py::arg("q1_fidelity") = 0, py::arg("readout_length") = 0, "Set properties relevant for quantum error modeling and backend calibration");

    py::class_<QuantumBackend, std::unique_ptr<QuantumBackend, py::nodelete>, Component>(m, "QuantumBackend")
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name") = "QuantumBackend")
        .def(py::init<Component *, int, std::string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "QuantumBackend")
        .def_property("num_qubits", &QuantumBackend::GetNumQubits, &QuantumBackend::SetNumQubits)
        .def("GetAllGateTypes", &QuantumBackend::GetAllGateTypes, "Get a list of the quantum gates in the backend")
        #ifdef QDMI
        .def_property("device", &QuantumBackend::GetQDMIDevice, &QuantumBackend::SetQDMIDevice)
        #endif
        .def("addGate", &QuantumBackend::addGate, py::arg("gate"), "Add this gate to the backend")
        .def("GetGatesBySize", &QuantumBackend::GetGatesBySize, py::arg("size"), "Get quantum gates by their size")
        .def("GetGatesByType", &QuantumBackend::GetGatesByType, py::arg("type"), "Get quantum gates by their type")
        .def("GetNumberofGates", &QuantumBackend::GetNumberofGates, "Get the number of gates in the backend")
        .def("GetAllQubits", &QuantumBackend::GetAllQubits, "Get all qubits in the backend");

    py::class_<AtomSite::SiteProperties>(m, "SiteProperties")
        .def_readwrite("nRows", &AtomSite::SiteProperties::nRows)
        .def_readwrite("nColumns", &AtomSite::SiteProperties::nColumns)
        .def_readwrite("nAods", &AtomSite::SiteProperties::nAods)
        .def_readwrite("nAodIntermediateLevels", &AtomSite::SiteProperties::nAodIntermediateLevels)
        .def_readwrite("nAodCoordinates", &AtomSite::SiteProperties::nAodCoordinates)
        .def_readwrite("interQubitDistance", &AtomSite::SiteProperties::interQubitDistance)
        .def_readwrite("interactionRadius", &AtomSite::SiteProperties::interactionRadius)
        .def_readwrite("blockingFactor", &AtomSite::SiteProperties::blockingFactor);
    py::class_<AtomSite, std::unique_ptr<AtomSite, py::nodelete>, QuantumBackend>(m, "AtomSite")
        .def(py::init<>())
        .def_readwrite("properties", &AtomSite::properties)
        .def_readwrite("shuttlingTimes", &AtomSite::shuttlingTimes)
        .def_readwrite("shuttlingAverageFidelities", &AtomSite::shuttlingAverageFidelities);

    py::class_<Relation, std::unique_ptr<Relation, py::nodelete>>(m, "Relation")
        .def(py::init<const std::vector<Component*> &, int, bool>(), py::arg("components"), py::arg("id") = 0, py::arg("ordered") = true)
        .def_property("id", &Relation::GetId, &Relation::SetId)
        .def_property_readonly("type", &Relation::GetType)
        .def_property_readonly("ordered", &Relation::IsOrdered)
        .def_property_readonly("components", &Relation::GetComponents)
        .def("__setitem__", [](Relation& self, const std::string& name, py::object value) {
            set_attribute<Relation>(self,name, value);
        })
        .def("__getitem__", [](Relation& self, const std::string& name) {
            return get_attribute<Relation>(self,name);
        })
        .def("__getitem__", [](Relation& self, int pos) {
            return get_attribute<Relation>(self,pos);
        })
        .def("__delitem__", [](Relation& self, const std::string& name) {
            remove_attribute<Relation>(self,name);
        })
        .def("GetTypeStr", &Relation::GetTypeStr, "Get a string representing the type of the relation")
        .def("ContainsComponent", &Relation::ContainsComponent, py::arg("component"), "Check if a component is part of this relation")
        .def("GetComponent", &Relation::GetComponent, py::arg("index"), "Get a component at a specific position")
        .def("Print", &Relation::Print, "Prin basic information about this relation")
        .def("AddComponent", &Relation::AddComponent, py::arg("component"), "Add this component to the relation")
        .def("UpdateComponent", (int (Relation::*) (int, Component *)) &Relation::UpdateComponent, py::arg("index"), py::arg("new_component"), "Tries to replace the component at the given index with the new component")
        .def("UpdateComponent", (int (Relation::*) (Component *, Component *)) &Relation::UpdateComponent, py::arg("old_component"), py::arg("new_component"), "Tries to find the old component to replace it with the new component")
        .def("Delete", &Relation::Delete, "Delete this relation");

    py::class_<DataPath, std::unique_ptr<DataPath, py::nodelete>, Relation>(m,"DataPath")
        .def(py::init<Component*, Component*, DataPathOrientation::type, DataPathType::type>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type") = sys_sage::DataPathType::None)
        .def(py::init<Component*, Component*, DataPathOrientation::type, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("bw"), py::arg("latency"))
        .def(py::init<Component*, Component*, DataPathOrientation::type, DataPathType::type, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type"), py::arg("bw"), py::arg("latency"))
        .def_property("bandwidth", &DataPath::GetBandwidth, &DataPath::SetBandwidth, "The bandwidth of the data path")
        .def_property("latency", &DataPath::GetLatency, &DataPath::SetLatency, "The latency of the data path")
        .def_property_readonly("dp_type", &DataPath::GetDataPathType, "The type of the data path")
        .def_property_readonly("orientation", &DataPath::GetOrientation, "The orientation of the data path")
        .def_property("source", &DataPath::GetSource, &DataPath::UpdateSource, "The source of the data path")
        .def_property("target", &DataPath::GetTarget, &DataPath::UpdateTarget, "The target of the data path")
        .def("Print", &DataPath::Print, "Print basic information of the data path to stdout")
        .def("Delete", &DataPath::Delete, "Delete the data path");

    py::class_<CouplingMap, std::unique_ptr<CouplingMap, py::nodelete>, Relation>(m, "CouplingMap")
        .def(py::init<Qubit *, Qubit *>(), py::arg("q1"), py::arg("q2"))
        .def(py::init<const std::vector<Component*>&, int, bool>(), py::arg("components"), py::arg("id") = 0, py::arg("ordered") = false)
        .def_property("fidelity", &CouplingMap::GetFidelity, &CouplingMap::SetFidelity)
        .def("Delete", &CouplingMap::Delete, "Delete the coupling map");

    py::class_<QuantumGate, std::unique_ptr<QuantumGate, py::nodelete>, Relation>(m, "QuantumGate")
        .def(py::init<size_t, std::string, double, std::string>(), py::arg("size") = 0, py::arg("name") = "", py::arg("fidelity") = 0.0, py::arg("unitary") = "")
        .def(py::init<size_t, const std::vector<Qubit *> &>(), py::arg("size"), py::arg("qubits"))
        .def(py::init<size_t, const std::vector<Qubit *> &, std::string, double, std::string>(), py::arg("size"), py::arg("qubits"), py::arg("name"), py::arg("fidelity"), py::arg("unitary"))
        .def(py::init<const std::vector<Component *> &, int, bool, size_t, std::string, int, QuantumGateType::type, double, std::string>(), py::arg("components"), py::arg("id") = 0, py::arg("ordered") = true, py::arg("size") = 0, py::arg("name") = "", py::arg("length") = 0, py::arg("type") = QuantumGateType::Unknown, py::arg("fidelity") = 0, py::arg("unitary") = "")
        .def_property("gate_size", &QuantumGate::GetGateSize, &QuantumGate::SetGateSize)
        .def_property("name", &QuantumGate::GetName, &QuantumGate::SetName)
        .def_property("gate_length", &QuantumGate::GetGateLength, &QuantumGate::SetGateLength)
        .def_property("gate_type", &QuantumGate::GetQuantumGateType, &QuantumGate::SetQuantumGateType)
        .def_property("fidelity", &QuantumGate::GetFidelity, &QuantumGate::SetFidelity)
        .def_property("unitary", &QuantumGate::GetUnitary, &QuantumGate::SetUnitary)
        .def("SetGateProperties", &QuantumGate::SetGateProperties, py::arg("name"), py::arg("fidelity"), py::arg("unitary"), "Sets the name, fidelity, unitary and type of the quantum gate")
        .def("Print", &QuantumGate::Print, "Print basic information about the quantum gate to stdout");

    m.def("parseMt4gTopo", (int (*) (Node*,std::string,int, std::string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Component*,std::string,int, std::string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Chip*,std::string, std::string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"),  py::arg("delim") = ";");

    m.def("parseHwlocOutput", &parseHwlocOutput, "parseHwlocOutput", py::arg("root"), py::arg("xmlPath"));

    m.def("parseCccbenchOutput", &parseCccbenchOutput, "parseCccbenchOutput", py::arg("root"), py::arg("cccPath"));

    m.def("parseCapsNumaBenchmark", &parseCapsNumaBenchmark,  py::arg("root"), py::arg("benchmarkPath"), py::arg("delim") = ";");

    m.def("parseIQM", (int (*) (Component *, std::string, int, int)) &parseIQM, "parseIQM", py::arg("parent"), py::arg("dataSourcePath"), py::arg("qcId"), py::arg("tsForHistory") = -1);
    m.def("parseIQM", (int (*) (QuantumBackend *, std::string, int, int, bool)) &parseIQM, "parseIQM", py::arg("parent"), py::arg("dataSourcePath"), py::arg("qcId"), py::arg("tsForHistory") = -1, py::arg("createTopo") = true);

    // TODO: QDMI parser logic is missing in src/parsers/qdmi-parser.hpp

    m.def("exportToXml", [](Component& root, std::string xmlPath, std::optional<py::function> print_att = std::nullopt, std::optional<py::function> print_catt = std::nullopt) {
        if(print_att)
            print_attributes = *print_att;
        if(print_catt)
            print_complex_attributes = *print_catt;
        exportToXml(&root, xmlPath,print_att ? xmldumper : nullptr,print_catt ? xmldumper_complex : nullptr);
    },py::arg("root"), py::arg("xmlPath") = "", py::arg("print_att") = py::none(), py::arg("print_catt") = py::none());
    
    m.def("importFromXml",[](std::string path, std::optional<py::function> search_custom_attrib_key_fcn = std::nullopt, std::optional<py::function> search_custom_complex_attrib_key_fcn = std::nullopt) {
        if(search_custom_attrib_key_fcn)
            read_attributes = *search_custom_attrib_key_fcn;
        if(search_custom_complex_attrib_key_fcn)
            read_complex_attributes = *search_custom_complex_attrib_key_fcn;
        return importFromXml(path,search_custom_attrib_key_fcn ? xmlloader : nullptr, search_custom_complex_attrib_key_fcn ? xmlloader_complex : nullptr );
    }, py::arg("path"), py::arg("search_custom_attrib_key_fcn") = py::none(), py::arg("search_custom_complex_attrib_key_fcn") = py::none());
}


#endif //PY_SYS_SAGE
#endif //SYS_SAGE_BINDINGS_CPP

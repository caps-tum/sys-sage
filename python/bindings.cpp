#include <cstdio>
#include <exception>
#include <libxml2/libxml/parser.h>
#include <optional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/attr.h>
#include <string>
#include <tuple>

#include <sys-sage.hpp>
#include <sys-sage/defines.hpp>

namespace py = pybind11;

std::vector<std::string> default_attribs = {"CATcos","CATL3mask","mig_size","Number_of_streaming_multiprocessors","Number_of_cores_in_GPU","Number_of_cores_per_SM","Bus_Width_bit","Clock_Frequency","latency","latency_min","latency_max","CUDA_compute_capability","mig_uuid","freq_history","GPU_Clock_Rate"};

py::function print_attributes;
py::function print_complex_attributes;

py::function read_attributes;
py::function read_complex_attributes;

// wrapper class to allow attributes iteration in Python
template <typename T>
class AttributeIteratorWrapper {
public:
    AttributeIteratorWrapper(T &_self, T::attribIterator _cur) : self (_self), cur (_cur) {}

    py::object next()
    {
        if (cur == self.AttributesEnd())
            throw py::stop_iteration();

        auto &key = cur->first;
        auto value = self.template GetAttribute<py::object>(cur++);

        return py::make_tuple(key, *value);
    }

private:
    T &self;
    T::attribIterator cur;
};

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

PYBIND11_MODULE(_py_sys_sage, m) {
    using namespace sys_sage;

    // expose the options of the library's build configuration

#ifdef INTEL_PQOS
    m.attr("HAS_INTEL_PQOS") = true;
#else
    m.attr("HAS_INTEL_PQOS") = false;
#endif

#ifdef NVIDIA_MIG
    m.attr("HAS_NVIDIA_MIG") = true;
#else
    m.attr("HAS_NVIDIA_MIG") = false;
#endif

#ifdef PROC_CPUINFO
    m.attr("HAS_PROC_CPUINFO") = true;
#else
    m.attr("HAS_PROC_CPUINFO") = false;
#endif

#ifdef DS_HWLOC
    m.attr("HAS_DS_HWLOC") = true;
#else
    m.attr("HAS_DS_HWLOC") = false;
#endif

#ifdef DS_MT4G
    m.attr("HAS_DS_MT4G") = true;
#else
    m.attr("HAS_DS_MT4G") = false;
#endif

#ifdef DS_NUMA
    m.attr("HAS_DS_NUMA") = true;
#else
    m.attr("HAS_DS_NUMA") = false;
#endif

#ifdef QDMI
    m.attr("HAS_QDMI") = true;
#else
    m.attr("HAS_QDMI") = false;
#endif

#ifdef SS_PAPI
    m.attr("HAS_PAPI") = true;
#else
    m.attr("HAS_PAPI") = false;
#endif

    m.attr("COMPONENT_ANY") = ComponentType::Any;
    m.attr("COMPONENT_GENERIC") = ComponentType::Generic;
    m.attr("COMPONENT_NONE") = ComponentType::Generic;
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

    m.attr("SUBDIVISION_CATEGORY_NONE") = SubdivisionCategory::None;
    m.attr("SUBDIVISION_CATEGORY_GPU_SM") = SubdivisionCategory::GpuSM;

    m.attr("CHIP_CATEGORY_NONE") = ChipCategory::None;
    m.attr("CHIP_CATEGORY_CPU") = ChipCategory::Cpu;
    m.attr("CHIP_CATEGORY_CPU_SOCKET") = ChipCategory::CpuSocket;
    m.attr("CHIP_CATEGORY_GPU") = ChipCategory::Gpu;

    m.attr("RELATION_TYPE_ANY") = RelationType::Any;
    m.attr("RELATION_TYPE_RELATION") = RelationType::Relation;
    m.attr("RELATION_TYPE_DATAPATH") = RelationType::DataPath;
    m.attr("RELATION_TYPE_QUANTUMGATE") = RelationType::QuantumGate;
    m.attr("RELATION_TYPE_COUPLINGMAP") = RelationType::CouplingMap;

    m.attr("RELATION_CATEGORY_ANY") = RelationCategory::Any;
    m.attr("RELATION_CATEGORY_DEFAULT") = RelationCategory::Default;
#ifdef SS_PAPI
    m.attr("RELATION_CATEGORY_PAPI_METRICS") = RelationCategory::PAPI_Metrics;
#endif

    m.attr("DATAPATH_CATEGORY_ANY") = DataPathCategory::Any;
    m.attr("DATAPATH_CATEGORY_NONE") = DataPathCategory::None;
    m.attr("DATAPATH_CATEGORY_LOGICAL") = DataPathCategory::Logical;
    m.attr("DATAPATH_CATEGORY_PHYSICAL") = DataPathCategory::Physical;
    m.attr("DATAPATH_CATEGORY_DATATRANSFER") = DataPathCategory::Datatransfer;
    m.attr("DATAPATH_CATEGORY_L3CAT") = DataPathCategory::L3CAT;
    m.attr("DATAPATH_CATEGORY_MIG") = DataPathCategory::MIG;
    m.attr("DATAPATH_CATEGORY_C2C") = DataPathCategory::C2C;

    m.attr("DATAPATH_DIRECTION_ANY") = DataPathDirection::Any;
    m.attr("DATAPATH_DIRECTION_OUTGOING") = DataPathDirection::Outgoing;
    m.attr("DATAPATH_DIRECTION_INCOMING") = DataPathDirection::Incoming;

    m.attr("DATAPATH_ORIENTATION_ORIENTED") = DataPathOrientation::Oriented;
    m.attr("DATAPATH_ORIENTATION_BIDIRECTIONAL") = DataPathOrientation::Bidirectional;

    m.attr("QUANTUMGATE_CATEGORY_UNKNOWN") = QuantumGateCategory::Unknown;
    m.attr("QUANTUMGATE_CATEGORY_ID") = QuantumGateCategory::Id;
    m.attr("QUANTUMGATE_CATEGORY_X") = QuantumGateCategory::X;
    m.attr("QUANTUMGATE_CATEGORY_RZ") = QuantumGateCategory::Rz;
    m.attr("QUANTUMGATE_CATEGORY_CNOT") = QuantumGateCategory::Cnot;
    m.attr("QUANTUMGATE_CATEGORY_SX") = QuantumGateCategory::Sx;
    m.attr("QUANTUMGATE_CATEGORY_TOFFOLI") = QuantumGateCategory::Toffoli;

    py::class_<AttributeIteratorWrapper<Component>>(m, "ComponentAttributeIterator")
        .def("__iter__", [](AttributeIteratorWrapper<Component> &self) -> AttributeIteratorWrapper<Component> & { return self; })
        .def("__next__", &AttributeIteratorWrapper<Component>::next);

    py::class_<AttributeIteratorWrapper<Relation>>(m, "RelationAttributeIterator")
        .def("__iter__", [](AttributeIteratorWrapper<Relation> &self) -> AttributeIteratorWrapper<Relation> & { return self; })
        .def("__next__", &AttributeIteratorWrapper<Relation>::next);

    //bind component class
    py::class_<Component, std::unique_ptr<Component, py::nodelete>>(m, "Component")
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name") = "unknown")
        .def(py::init<Component *, int, std::string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "unknown")
        /*
         * The iterators and methods that take an iterator as a parameter are
         * not exposed since there is no 1-to-1 correspondance of an iterator
         * in Python. Normal iteration inside a for-loop is enabled through a
         * helper class. Keys should be used instead of iterators like in
         * `Component::GetAttribute(attribIterator it)`.
         * 
         * Furthermore, the method `Component::UpdateAttribute` is not exposed
         * since the underlying `py::object` needs to be freed and a new one
         * needs to be allocated anyways.
         *
         * JSON serialization for attributes in Python can be supported through
         * https://github.com/pybind/pybind11_json
         * However, this uses `nlohmann::json` while we use
         * `nlohmann::ordered_json`. -> TODO: how to fix this?
         *
         * All of the above also holds for the `Relation` class.
         */
        .def("SetAttribute", [](Component &self, const std::string &key, py::object value) -> py::object { return *self.SetAttribute<py::object>(key, std::move(value)); })
        .def("GetAttribute", [](Component &self, const std::string &key) -> py::object
            {
                auto value = self.GetAttribute<py::object>(key);
                return value ? *value : py::none();
            }
        )
        .def("GetAttributesSize", &Component::GetAttributesSize)
        .def("IterateAttributes", [](Component &self) -> AttributeIteratorWrapper<Component> { return AttributeIteratorWrapper(self, self.AttributesBegin()); })
        .def("EraseAttribute", (void (Component::*)(const std::string &))(&Component::EraseAttribute), py::arg("key"))
        .def("ClearAttributes", &Component::ClearAttributes)
        .def("InsertChild", &Component::InsertChild, py::arg("child"), "Insert a child component")
        .def("InsertBetweenParentAndChild", &Component::InsertBetweenParentAndChild, py::arg("parent"), py::arg("child"), py::arg("alreadyParentsChild"),"Insert a component between parent and child")
        .def("InsertBetweenParentAndChildren", &Component::InsertBetweenParentAndChildren, py::arg("parent"), py::arg("children"), py::arg("alreadyParentsChildren"), "Insert a component between parent and children")
        .def("RemoveChild", &Component::RemoveChild, py::arg("child"),"Remove a child component")
        .def_property("parent", &Component::GetParent, &Component::SetParent, "The parent of the component")
        .def("SetParent", &Component::SetParent, py::arg("parent"), "Set the parent of the component")
        .def("PrintSubtree", &Component::PrintSubtree, "Print the subtree of the component up to level 0")
        .def("PrintRelationsInSubtree", &Component::PrintRelationsInSubtree, py::arg("relationType") = RelationType::Any, "Print the relations in the subtree")
        .def_property("name", &Component::GetName, &Component::SetName, "The name of the component")
        .def_property_readonly("id", &Component::GetId, "The id of the component")
        .def_property_readonly("type", &Component::GetComponentType, "The type of the component")
        .def("GetComponentTypeStr", &Component::GetComponentTypeStr, "The type of the component as string")
        .def("GetChildren", &Component::GetChildren, "The children of the component")
        .def("GetChild", &Component::GetChild, py::arg("id"), "Like get_child_by_id()")
        .def("GetChildById", &Component::GetChildById, py::arg("id"), "Get the first child component by id")
        .def("GetChildByType", &Component::GetChildByType, py::arg("type"), "Get the first child component by type")
        .def("FindChildrenByType", (std::vector<Component*> (Component::*)(ComponentType::type) const)(&Component::FindChildrenByType), py::arg("type"), "Find the child components by type")
        .def("FindDescendantsByType", (std::vector<Component*> (Component::*) (ComponentType::type)) (&Component::FindDescendantsByType), py::arg("type") , "Find descendants by type")
        .def("CountDescendantsByType", &Component::CountDescendantsByType, py::arg("type"), "Count the descendants by type")
        .def("CountChildrenByType", &Component::CountChildrenByType, py::arg("type"), "Count children by type")
        .def("GetAncestorByType", &Component::GetAncestorByType, py::arg("type"),"Get the first ancestor component by type")
        .def("CalcSubtreeDepth", &Component::CalcSubtreeDepth, "Calculate the depth of the subtree")
        .def("GetNthAncestor", &Component::GetNthAncestor, py::arg("n"),"Get the nth ancestor of the component")
        .def("FindNthDescendants", (std::vector<Component*> (Component::*) (int)) &Component::FindNthDescendants, py::arg("n"), "Find the nth descendants of the component")
        .def("GetDescendantById", &Component::GetDescendantById, py::arg("id"), py::arg("type"), "Get the first descendant by id and type")
        .def("GetRelationsByType", &Component::GetRelationsByType, py::arg("type"), "Get all relations of that type")
        .def("FindRelations", &Component::FindRelations, py::arg("type") = RelationType::Any, py::arg("position") = -1, "Find the relations of that type and position")
        .def("GetDataPathByCategory", &Component::GetDataPathByCategory, py::arg("category"), py::arg("direction") = DataPathDirection::Any,"Get the first data path associated with the component by type")
        .def("FindDataPaths", (std::vector<DataPath *> (Component::*) (DataPathCategory::type, DataPathDirection::type) const) &Component::FindDataPaths, py::arg("category") = DataPathCategory::Any, py::arg("direction") = DataPathDirection::Any, "Find the datapaths of that category and direction")
        .def("CheckSubtreeConsistency", &Component::CheckSubtreeConsistency, "Check if the subtree is consistent")
        // pybind11 doesn't support pass-by-reference or pass-by-pointer of primitive types.
        // -> use a tuple instead of output parameters
        .def("CalcSubtreeSize", [] (Component &self) {
            unsigned out_component_size = 0;
            unsigned out_dataPathSize = 0;
            int total_bytes = self.CalcSubtreeSize(&out_component_size, &out_dataPathSize);
            return std::make_tuple(total_bytes, out_component_size, out_dataPathSize);
        }, "Calculate the size of the subtree")
        .def("CalcDepth", &Component::CalcDepth, py::arg("refresh"), "Calculate the depth of the component, if refresh is true it will update the depth")
        .def("DeleteRelations", &Component::DeleteRelations, py::arg("type") = RelationType::Any, "Delete the relations of that type from the component")
#ifdef SS_PAPI
        .def("PrintPAPImetricsInSubtree", &Component::PrintPAPImetricsInSubtree, py::arg("eventSet") = PAPI_NULL)
        .def("FindPAPIrelationsInSubtree", (std::vector<Relation *> (Component::*)() const) &Component::FindPAPIrelationsInSubtree)
#endif
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
        .def(py::init<int,std::string,ChipCategory::type,std::string,std::string>(), py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType")= ChipCategory::None, py::arg("vendor") = "", py::arg("model") = "")
        .def(py::init<Component*,int,std::string,ChipCategory::type,std::string,std::string>(), py::arg("parent"),py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType") = ChipCategory::None, py::arg("vendor") = "", py::arg("model") = "")
        .def_property("vendor", &Chip::GetVendor, &Chip::SetVendor, "The vendor of the chip")
        .def_property("model", &Chip::GetModel, &Chip::SetModel, "The model of the chip")
        .def_property("chipType", &Chip::GetChipCategory, &Chip::SetChipCategory, "The type of the chip");

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
        .def_property("subdivisionType", &Subdivision::GetSubdivisionCategory, &Subdivision::SetSubdivisionCategory, "The type of the subdivision");

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
#ifdef SS_PAPI
        .def("GetPAPImetric", &Thread::GetPAPImetric, py::arg("eventCode"), py::arg("eventSet"), py::arg("timestamp") = 0)
        .def("PrintPAPImetrics", &Thread::PrintPAPImetrics, py::arg("eventSet") = PAPI_NULL)
        .def("GetPAPIrelation", &Thread::PrintPAPImetrics, py::arg("eventSet"))
        .def("FindPAPIrelations", (std::vector<Relation *> (Thread::*)() const) &Thread::FindPAPIrelations)
        .def("FindPAPIeventSets", (std::vector<int> (Thread::*)() const) &Thread::FindPAPIeventSets)
#endif
        .def(py::init<int,std::string>(),py::arg("id") = 0,py::arg("name") = "Thread")
        .def(py::init<Component*,int,std::string>(),py::arg("parent"),py::arg("id") = 0,py::arg("name") = "Thread");

    py::class_<Qubit, std::unique_ptr<Qubit, py::nodelete>, Component>(m, "Qubit")
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name") = "Qubit")
        .def(py::init<Component *, int, std::string> (), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Qubit")
        #ifdef QDMI
        .def("RefreshProperties", &Qubit::RefreshProperties)
        #endif
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
        .def("RefreshTopology", &QuantumBackend::RefreshTopology, py::arg("qubit_indices"), "Get all qubits in the backend")
        #endif
        .def("addGate", &QuantumBackend::addGate, py::arg("gate"), "Add this gate to the backend")
        .def("FindGatesBySize", &QuantumBackend::FindGatesBySize, py::arg("size"), "Find quantum gates by their size")
        .def("FindGatesByType", &QuantumBackend::FindGatesByType, py::arg("type"), "Find quantum gates by their type")
        .def("GetNumberofGates", &QuantumBackend::GetNumberofGates, "Get the number of gates in the backend")
        .def("FindAllQubits", &QuantumBackend::FindAllQubits, "Find all qubits in the backend");

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
        .def_property_readonly("category", &Relation::GetCategory)
        .def_property_readonly("ordered", &Relation::IsOrdered)
        .def_property_readonly("components", &Relation::GetComponents)
        .def("SetAttribute", [](Relation &self, const std::string &key, py::object value) -> py::object { return *self.SetAttribute<py::object>(key, std::move(value)); })
        .def("GetAttribute", [](Relation &self, const std::string &key) -> py::object
            {
                auto value = self.GetAttribute<py::object>(key);
                return value ? *value : py::none();
            }
        )
        .def("GetAttributesSize", &Relation::GetAttributesSize)
        .def("IterateAttributes", [](Relation &self) -> AttributeIteratorWrapper<Relation> { return AttributeIteratorWrapper(self, self.AttributesBegin()); })
        .def("EraseAttribute", (void (Relation::*)(const std::string &))(&Relation::EraseAttribute), py::arg("key"))
        .def("ClearAttributes", &Relation::ClearAttributes)
#ifdef SS_PAPI
        .def("GetPAPImetric", &Relation::GetPAPImetric, py::arg("eventCode"), py::arg("cpuNum") = -1, py::arg("timestamp") = 0)
        .def("GetAllPAPImetrics", &Relation::GetAllPAPImetrics, py::arg("eventCode"), py::arg("cpuNum"))
        .def("PrintPAPImetrics", &Relation::PrintPAPImetrics, py::arg("cpuNum") = -1)
        .def("FindPAPIevents", (std::vector<int> (Relation::*)() const) &Relation::FindPAPIevents)
        .def("GetCurrentEventSet", &Relation::GetCurrentEventSet)
        .def("GetElapsedTime", &Relation::GetElapsedTime, py::arg("timestamp"))
        .def("GetLatestCpuNum", &Relation::GetLatestCpuNum)
#endif
        .def("GetTypeStr", &Relation::GetTypeStr, "Get a string representing the type of the relation")
        .def("ContainsComponent", &Relation::ContainsComponent, py::arg("component"), "Check if a component is part of this relation")
        .def("GetComponent", &Relation::GetComponent, py::arg("index"), "Get a component at a specific position")
        .def("Print", &Relation::Print, "Prin basic information about this relation")
        .def("AddComponent", &Relation::AddComponent, py::arg("component"), "Add this component to the relation")
        .def("RemoveComponent", (int (Relation::*) (size_t)) &Relation::RemoveComponent, py::arg("index"), "Removes the component at the given index")
        .def("RemoveComponent", (int (Relation::*) (Component *)) &Relation::RemoveComponent, py::arg("component"), "Removes the given component from the relation")
        .def("UpdateComponent", (int (Relation::*) (int, Component *)) &Relation::UpdateComponent, py::arg("index"), py::arg("new_component"), "Tries to replace the component at the given index with the new component")
        .def("UpdateComponent", (int (Relation::*) (Component *, Component *)) &Relation::UpdateComponent, py::arg("old_component"), py::arg("new_component"), "Tries to find the old component to replace it with the new component");

    py::class_<DataPath, std::unique_ptr<DataPath, py::nodelete>, Relation>(m,"DataPath")
        .def(py::init<Component*, Component*, DataPathOrientation::type, DataPathCategory::type>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type") = sys_sage::DataPathCategory::None)
        .def(py::init<Component*, Component*, DataPathOrientation::type, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("bw"), py::arg("latency"))
        .def(py::init<Component*, Component*, DataPathOrientation::type, DataPathCategory::type, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type"), py::arg("bw"), py::arg("latency"))
        .def_property("bandwidth", &DataPath::GetBandwidth, &DataPath::SetBandwidth, "The bandwidth of the data path")
        .def_property("latency", &DataPath::GetLatency, &DataPath::SetLatency, "The latency of the data path")
        .def_property_readonly("dp_category", &DataPath::GetDataPathCategory, "The type of the data path")
        .def_property_readonly("orientation", &DataPath::GetOrientation, "The orientation of the data path")
        .def_property("source", &DataPath::GetSource, &DataPath::UpdateSource, "The source of the data path")
        .def_property("target", &DataPath::GetTarget, &DataPath::UpdateTarget, "The target of the data path")
        .def("Print", &DataPath::Print, "Print basic information of the data path to stdout");

    py::class_<CouplingMap, std::unique_ptr<CouplingMap, py::nodelete>, Relation>(m, "CouplingMap")
        .def(py::init<Qubit *, Qubit *>(), py::arg("q1"), py::arg("q2"))
        .def(py::init<const std::vector<Component*>&, int, bool>(), py::arg("components"), py::arg("id") = 0, py::arg("ordered") = false)
        .def_property("fidelity", &CouplingMap::GetFidelity, &CouplingMap::SetFidelity);

    py::class_<QuantumGate, std::unique_ptr<QuantumGate, py::nodelete>, Relation>(m, "QuantumGate")
        .def(py::init<size_t, std::string, double, std::string>(), py::arg("size") = 0, py::arg("name") = "", py::arg("fidelity") = 0.0, py::arg("unitary") = "")
        .def(py::init<size_t, const std::vector<Qubit *> &>(), py::arg("size"), py::arg("qubits"))
        .def(py::init<size_t, const std::vector<Qubit *> &, std::string, double, std::string>(), py::arg("size"), py::arg("qubits"), py::arg("name"), py::arg("fidelity"), py::arg("unitary"))
        .def(py::init<const std::vector<Component *> &, int, bool, size_t, std::string, int, QuantumGateCategory::type, double, std::string>(), py::arg("components"), py::arg("id") = 0, py::arg("ordered") = true, py::arg("size") = 0, py::arg("name") = "", py::arg("length") = 0, py::arg("type") = QuantumGateCategory::Unknown, py::arg("fidelity") = 0, py::arg("unitary") = "")
        .def_property("gate_size", &QuantumGate::GetGateSize, &QuantumGate::SetGateSize)
        .def_property("name", &QuantumGate::GetName, &QuantumGate::SetName)
        .def_property("gate_length", &QuantumGate::GetGateLength, &QuantumGate::SetGateLength)
        .def_property("gate_type", &QuantumGate::GetQuantumGateCategory, &QuantumGate::SetQuantumGateCategory)
        .def_property("fidelity", &QuantumGate::GetFidelity, &QuantumGate::SetFidelity)
        .def_property("unitary", &QuantumGate::GetUnitary, &QuantumGate::SetUnitary)
        .def("SetGateProperties", &QuantumGate::SetGateProperties, py::arg("name"), py::arg("fidelity"), py::arg("unitary"), "Sets the name, fidelity, unitary and type of the quantum gate")
        .def("Print", &QuantumGate::Print, "Print basic information about the quantum gate to stdout");

    m.def("ParseMt4g", (int (*) (Component *, const std::string &, int)) &ParseMt4g, py::arg("parent"), py::arg("path"), py::arg("gpuId"), "Construct a complete GPU topology by parsing an mt4g output file.");
    m.def("ParseMt4g_v1_x", (int (*) (Component *, const std::string &, int)) &ParseMt4g_v1_x, py::arg("parent"), py::arg("path"), py::arg("gpuId"), "Construct a complete GPU topology by parsing an mt4g output file.");
    m.def("ParseMt4g_v0_1", (int (*) (Component *, const std::string &, int, const std::string)) &ParseMt4g_v0_1, py::arg("parent"), py::arg("path"), py::arg("gpuId"), py::arg("delim") = ";", "Construct a complete GPU topology by parsing an mt4g output file.");

    m.def("parseHwlocOutput", &parseHwlocOutput, "parseHwlocOutput", py::arg("root"), py::arg("xmlPath"));

    m.def("parseCccbenchOutput", &parseCccbenchOutput, "parseCccbenchOutput", py::arg("root"), py::arg("cccPath"));

    m.def("parseCapsNumaBenchmark", &parseCapsNumaBenchmark,  py::arg("root"), py::arg("benchmarkPath"), py::arg("delim") = ";");

    m.def("parseIQM", (int (*) (Component *, const std::string &, int, int)) &parseIQM, "parseIQM", py::arg("parent"), py::arg("dataSourcePath"), py::arg("qcId"), py::arg("tsForHistory") = -1);
    m.def("parseIQM", (int (*) (QuantumBackend *, const std::string&, int, int, bool)) &parseIQM, "parseIQM", py::arg("parent"), py::arg("dataSourcePath"), py::arg("qcId"), py::arg("tsForHistory") = -1, py::arg("createTopo") = true);

    // TODO: QDMI parser logic is missing in src/external_interfaces/qdmi-parser.hpp

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

#ifdef SS_PAPI
    py::class_<Metric, std::unique_ptr<Metric, py::nodelete>>(m, "Metric")
      .def_readwrite("timestamp", &Metric::timestamp)
      .def_readwrite("value", &Metric::value)
      .def_readwrite("permanent", &Metric::permanent)

      .def("__str__", [](Metric &self){
        std::string str = "{ .timestamp = ";
        str += self.timestamp;
        str += ", .value = ";
        str += self.value;
        str += " }";

        return str;
      });

    py::class_<CpuMetrics, std::unique_ptr<CpuMetrics, py::nodelete>>(m, "CpuMetrics")
      .def_readwrite("entries", &CpuMetrics::entries)
      .def_readwrite("cpuNum", &CpuMetrics::cpuNum);

    m.def("SS_PAPI_start", [](int eventSet, Relation *metrics) {
      int rval = SS_PAPI_start(eventSet, &metrics);
      return std::make_tuple(rval, metrics);
    }, py::arg("eventSet"), py::arg("metrics"));

    m.def("SS_PAPI_reset", &SS_PAPI_reset, py::arg("metrics"));

    m.def("SS_PAPI_read", [](Relation *metrics, Component *root, bool permanent = false) {
      unsigned long long timestamp;
      int rval = SS_PAPI_read(metrics, root, permanent, &timestamp);

      return std::make_tuple(rval, timestamp);
    }, py::arg("metrics"), py::arg("root"), py::arg("permanent") = false);

    m.def("SS_PAPI_accum", [](Relation *metrics, Component *root, bool permanent = false) {
      unsigned long long timestamp;
      int rval = SS_PAPI_accum(metrics, root, permanent, &timestamp);

      return std::make_tuple(rval, timestamp);
    }, py::arg("metrics"), py::arg("root"), py::arg("permanent") = false);

    m.def("SS_PAPI_stop", [](Relation *metrics, Component *root, bool permanent = false) {
      unsigned long long timestamp;
      int rval = SS_PAPI_stop(metrics, root, permanent, &timestamp);

      return std::make_tuple(rval, timestamp);
    }, py::arg("metrics"), py::arg("root"), py::arg("permanent") = false);

#endif
}

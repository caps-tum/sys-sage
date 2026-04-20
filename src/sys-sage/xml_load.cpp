#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <vector>

#include "xml_load.hpp"

#include "Topology.hpp"
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
#include "AtomSite.hpp"
#include "Relation.hpp"
#include "DataPath.hpp"
#include "QuantumGate.hpp"
#include "CouplingMap.hpp"

using std::cout;
using std::endl;


#include <libxml/parser.h>

// Function pointer for custom attribute key search
std::function<void*(xmlNodePtr)> load_custom_attrib_fcn = NULL;
// Function pointer for custom complex attribute key search
std::function<int(xmlNodePtr, sys_sage::Component *)> load_custom_complex_attrib_fcn = NULL;

// Create map of addresses to Components created
// this is used to create the Datapaths
std::map<std::string, sys_sage::Component *> addr_to_component;

//Helper-Function to retrieve string from xml-node
std::string sys_sage::_getStringFromProp(xmlNodePtr n, std::string prop) {
	const unsigned char *v = xmlGetProp(n, reinterpret_cast<const unsigned char *>(prop.c_str()));
	std::string value(reinterpret_cast<char const *>(v));
	return value;
}

// Extract attribute value from xml-node based on attribute name
void* sys_sage::_search_default_attrib_key(xmlNodePtr n) {
	std::string key, value;
	//check if the node has a name-attribute
	if (xmlHasProp(n, BAD_CAST "name") && xmlHasProp(n, BAD_CAST "value")) 
	{
		key = _getStringFromProp(n, "name");
		value = _getStringFromProp(n, "value");
	}
	else 
	{
		return NULL;
	}


	// Handle attributes with uint64_t values
	if (!key.compare("CATcos") || !key.compare("CATL3mask")) 
	{
		return new long long (std::strtoull(reinterpret_cast<const char *>(value.c_str()), NULL, 16));
	}
	// Handle attributes with long long values
	else if (!key.compare("mig_size")) 
	{
		return new long long (std::strtoull(reinterpret_cast<const char *>(value.c_str()), NULL, 10));
	}

	// Handle attributes with int values
	else if (!key.compare("Number_of_streaming_multiprocessors") ||
		!key.compare("Number_of_cores_in_GPU") ||
		!key.compare("Number_of_cores_per_SM") ||
		!key.compare("Bus_Width_bit")) 
	{
		return (new int(std::stoi(value)));
	}

	// Handle attributes with double values
	else if (!key.compare("Clock_Frequency") || !key.compare("GPU_Clock_Rate")) 
	{
		return new double(std::stod(value));
	}

	// Handle attributes with float values
	else if (!key.compare("latency") || !key.compare("latency_min") ||
		!key.compare("latency_max"))
	{
		return new float(std::stof(value));
	}

	// Handle attributes with string values
	else if (!key.compare("CUDA_compute_capability") ||
		!key.compare("mig_uuid"))
	{
		return new std::string(value);
	}

	return NULL; // Attribute not found or not handled
}

// Search for custom complex attributes in xmlNode n and add them to Component c
//
// Complex attributes are attributes that have a value that is not a simple type
// but a more complex structure like a vector
int sys_sage::_search_default_complex_attrib_key(xmlNodePtr n, Component *c) {
	std::string key;
	if (xmlHasProp(n, BAD_CAST "name")) 
	{
		key = _getStringFromProp(n, "name");
	}
	else 
	{
		return 0;
	}
	// freq_history is a vector of tuples containing the timestamp and the
	// frequency
	if (!key.compare("freq_history")) {
		std::vector<std::tuple<long long, double>>* val = new std::vector<std::tuple<long long, double>>();
		for (xmlNodePtr cur = n->children; cur != NULL; cur = cur->next) 
		{
			// skip text nodes
			if (cur->type == XML_TEXT_NODE)
				continue;

			// add value to vector
			std::string ts = _getStringFromProp(cur, "timestamp");
			std::string freq = _getStringFromProp(cur, "frequency");
			long long ts_ll = std::strtoll((const char *)ts.c_str(), NULL, 10);
			double freq_d = std::stod(freq);

			val->push_back(std::make_tuple(ts_ll, freq_d));
		}
		c->attrib[key] = reinterpret_cast<void*>(val);
		return 1;
	} 
	//else if (!key.compare("GPU_Clock_Rate"))
	//{
	//	// GPU_Clock_Rate is a vector of tuples containing the frequency and the unit
	//	xmlNodePtr attr = n->children;
	//	if(attr->type == XML_TEXT_NODE)
	//		attr = attr->next;

	//	std::string unit = _getStringFromProp(attr, "unit");
	//	std::string freq = _getStringFromProp(attr, "frequency");

	//	double freq_d = std::stod(freq);

	//	auto val = new std::tuple<double, std::string>(freq_d, unit); 
	//	void *value = (void *)val;
	//	c->attrib[key] = value;

	//	return 1;
	//}

	return 0;
}

// Collect all attributes and add to Component c
//
// This function traverses the xmlNode n and extracts all attributes.
// The attributes are added to the Component c.
// If the custom functions are not null, they are called first. If they
// can not handle the attribute, the default functions are used.
int sys_sage::_collect_attrib(xmlNodePtr n, Component *c) {
	void *attrib_value = NULL;
	// try custom attribute search function
	if (load_custom_attrib_fcn != NULL)
		attrib_value = load_custom_attrib_fcn(n);
	// if custom function could not handle attribute, try default
	if (attrib_value == NULL)
		attrib_value = _search_default_attrib_key(n);
	// if attribute was handled, add it to Component
	if (attrib_value != NULL) {
		std::string key = _getStringFromProp(n, "name");
		c->attrib[key] = attrib_value;
	}
	int ret = 0;
	// try custom complex attribute search function
	if (attrib_value == NULL && load_custom_complex_attrib_fcn != NULL)
		ret = load_custom_complex_attrib_fcn(n, c);
	// if custom function could not handle attribute, try default
	if (attrib_value == NULL && ret == 0)
		return _search_default_complex_attrib_key(n, c);

	return 0;
}

// Create ComponentSubtree from xmlNodes
//
// This function creates a ComponentSubtree from the xmlNode n by creating
// a Component and then recursively calling itself for all children of n.
sys_sage::Component* sys_sage::_CreateComponentSubtree(xmlNodePtr n) {
	Component *c = NULL;

	std::string nodeName(reinterpret_cast<const char *>(n->name));
	// cout << "_CreateComponentSubtree nodeName = " << nodeName << endl;

	// entrypoint is 'Components' -- just scan for a non-'text' child and process that"
	if(nodeName.compare("Components") == 0)
	{
		for (xmlNodePtr xml_child = n->children; xml_child != NULL; xml_child = xml_child->next)
		{
			std::string childName(reinterpret_cast<const char *>(xml_child->name));
			if (childName.compare("text") == 0)
				continue;
			else
				return _CreateComponentSubtree(xml_child);
		}
	}

	//from here processing real Components

	std::string name = _getStringFromProp(n, "name");
	int id = std::stoi(_getStringFromProp(n, "id"));
	std::string addr = _getStringFromProp(n, "addr");

	// Check the type of Component and create the corresponding Component
	if (nodeName.compare("None") == 0) {
		c = new Component((id));
	}
	if (nodeName.compare("HW_Thread") == 0) {
		c = new Thread(id);
	}
	if (nodeName.compare("Core") == 0) {
		c = new Core(id);
	}
	if (nodeName.compare("Cache") == 0) {
		c = new Cache(id);
		if (xmlHasProp(n, BAD_CAST "cache_level")) {
			std::string value = _getStringFromProp(n, "cache_level");
			static_cast<Cache*>(c)->SetCacheLevel(std::stoi(value));
		}
		if (xmlHasProp(n, BAD_CAST "cache_size")) {
			std::string value = _getStringFromProp(n, "cache_size");
			static_cast<Cache*>(c)->SetCacheSize(std::stoi(value));
		}
		if (xmlHasProp(n, BAD_CAST "cache_associativity_ways")) {
			std::string value = _getStringFromProp(n, "cache_associativity_ways");
			static_cast<Cache*>(c)->SetCacheAssociativityWays(std::stoi(value));
		}
		if (xmlHasProp(n, BAD_CAST "cache_line_size")) {
			std::string value = _getStringFromProp(n, "cache_line_size");
			static_cast<Cache*>(c)->SetCacheLineSize(std::stoi(value));
		}
	}
	if (nodeName.compare("Subdivision") == 0) {
		c = new Subdivision(id);
		int sd_type = std::stoi(_getStringFromProp(n, "type"));

		static_cast<Subdivision*>(c)->SetSubdivisionType(sd_type);
	}
	if (nodeName.compare("NUMA") == 0) {
		c = new Numa(id);
		if (xmlHasProp(n, BAD_CAST "size")) {
			std::string value = _getStringFromProp(n, "size");
			static_cast<Numa*>(c)->SetSize(std::stoll(value));
		}
	}
	if (nodeName.compare("Chip") == 0) {
		c = new Chip(id);
		// check for vendor and model
		if (xmlHasProp(n, BAD_CAST "vendor")) {
			std::string value = _getStringFromProp(n, "vendor");
			static_cast<Chip*>(c)->SetVendor(value);
		}
		if (xmlHasProp(n, BAD_CAST "model")) {
			std::string model = _getStringFromProp(n, "model");
			static_cast<Chip*>(c)->SetModel(model);
		}
	}
	if (nodeName.compare("Memory") == 0) {
		long long size = 0;
		bool is_volatile = false;
		if (xmlHasProp(n, BAD_CAST "size")) {
			std::string value = _getStringFromProp(n, "size");
			size = std::stoll(value);
		}
		if (xmlHasProp(n, BAD_CAST "is_volatile")) {
			std::string value = _getStringFromProp(n, "is_volatile");
			is_volatile = (std::string(value) == "true");
		}
		c = new Memory(NULL, id, name, size, is_volatile);
	}
	if (nodeName.compare("Storage") == 0) {
		// Same as with memory
		long long size = 0;
		if (xmlHasProp(n, BAD_CAST "size")) {
			std::string value = _getStringFromProp(n, "size");
			size = std::stoll(value);
		}
		c = new Storage();
		static_cast<Storage *>(c)->SetSize(size);
	}
	if (nodeName.compare("Node") == 0) {
		c = new Node(id);
	}
	if (nodeName.compare("QuantumBackend") == 0) {
		c = new QuantumBackend(id);
	}
	if (nodeName.compare("Qubit") == 0) {
		c = new Qubit(id);
	}
	if (nodeName.compare("Topology") == 0) {
		c = new Topology();
	}

	// Recursively traverse all children of n and create Components
	for (xmlNodePtr xml_child = n->children; xml_child != NULL; xml_child = xml_child->next)
	{
		std::string childName(reinterpret_cast<const char *>(xml_child->name));

		// Skip Text nodes
		if (childName.compare("text") == 0)
			continue;
		// Check if cur is Attribute-Node
		if (childName.compare("Attribute") == 0) {
			_collect_attrib(xml_child, c);
		} else {
			Component *child = _CreateComponentSubtree(xml_child);
			// cout << "nodeName = " << nodeName << "   childName = " << childName << ", childPtr = " << child << endl;
			if (child != NULL) {
				c->InsertChild(child);
			}
		}
	}
	// Add the Component to the hashmap
	addr_to_component[addr] = c;
	return c;
}

// Create Relation objects from xmlNode dpNode and add them to the
// corresponding Components
int sys_sage::_CreateRelations(xmlNodePtr relationNode) {

	// cout << "_CreateRelations name = " << relationNode->name << endl; 
	for (xmlNodePtr xml_child = relationNode->children; xml_child != NULL; xml_child = xml_child->next) {

		// skip non-element nodes
		if (xml_child->type != XML_ELEMENT_NODE)
			continue;

		std::string childName(reinterpret_cast<const char *>(xml_child->name));
		// cout << "_CreateRelations childName = " << childName << endl;

    std::vector<Component *> components;
		xmlChar *str = xmlGetProp(xml_child, BAD_CAST "components");
    if (str != nullptr) {
        std::istringstream stream (reinterpret_cast<const char *>(str));
        std::string component;
        while (std::getline(stream, component, ' '))
          components.push_back(addr_to_component[component]);
    }

		std::string ordered_str = _getStringFromProp(xml_child, "ordered");
		bool ordered = (ordered_str == "1");
		int id = std::stoi(_getStringFromProp(xml_child, "id"));


		if (childName.compare("Relation") == 0)
		{
			new Relation(components, id, ordered);
		}
		if (childName.compare("DataPath") == 0)
		{
			int dataPathType = std::stoi(_getStringFromProp(xml_child, "DataPathType"));
			double bw = std::stod(_getStringFromProp(xml_child, "bw"));
			double latency = std::stod(_getStringFromProp(xml_child, "latency"));
			DataPathOrientation::type dpo = (ordered ? DataPathOrientation::Oriented : DataPathOrientation::Bidirectional);

			new DataPath(components[0], components[1], dpo, dataPathType, bw, latency);
		}
		if (childName.compare("QuantumGate") == 0)
		{
			int gate_size = std::stoi(_getStringFromProp(xml_child, "gate_size"));
			std::string name = _getStringFromProp(xml_child, "name");
			int gate_length = std::stoi(_getStringFromProp(xml_child, "gate_length"));
			QuantumGateType::type gate_type = std::stoi(_getStringFromProp(xml_child, "gate_type"));
			double fidelity = std::stod(_getStringFromProp(xml_child, "fidelity"));
			std::string unitary = _getStringFromProp(xml_child, "unitary");

			new QuantumGate(components, id, ordered, gate_size, name, gate_length, gate_type, fidelity, unitary);
		}
		if (childName.compare("CouplingMap") == 0)
		{
			CouplingMap* cm = new CouplingMap(components, id, ordered);

			double fidelity = std::stod(_getStringFromProp(xml_child, "fidelity"));
			cm->SetFidelity(fidelity);
		}		
	}
	return 1;
}

sys_sage::Component* sys_sage::importFromXml(
	std::string path,
	std::function<void*(xmlNodePtr)> _load_custom_attrib_fcn,
	std::function<int(xmlNodePtr, Component *)> _load_custom_complex_attrib_fcn) 
{

	load_custom_attrib_fcn = _load_custom_attrib_fcn;
	load_custom_complex_attrib_fcn = _load_custom_complex_attrib_fcn;

	xmlInitParser();
	xmlDocPtr doc = xmlReadFile(path.c_str(), NULL, 0);
	xmlNodePtr sys_sage_root = xmlDocGetRootElement(doc);
	// cout << "sys_sage_root->name = " << sys_sage_root->name << endl;

	Component *c = NULL;
	for (xmlNodePtr n = sys_sage_root->children; n != NULL; n = n->next) {
		if (xmlStrcmp(n->name, BAD_CAST "text") == 0) {
			//do nothing
		}
		else if (xmlStrcmp(n->name, BAD_CAST "Components") == 0) {
			c = _CreateComponentSubtree(n);
		}
		else if (xmlStrcmp(n->name, BAD_CAST "Relations") == 0) {
			_CreateRelations(n);
		}
	}

	return c;
	}


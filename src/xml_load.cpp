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

using namespace std;

#import <libxml/parser.h>

// Function pointer for custom attribute key search
std::function<void*(xmlNodePtr)> search_custom_attrib_key_fcn = NULL;

// Function pointer for custom complex attribute key search
std::function<int(xmlNodePtr, Component *)>
    search_custom_complex_attrib_key_fcn = NULL;

// Create map of addresses to Components created
std::map<string, Component *> addr_to_component;

//Helper-Function to retrieve string from xml-node
std::string getStringFromProp(xmlNodePtr n, string prop) {
  const unsigned char *v = xmlGetProp(n, (const unsigned char *)prop.c_str());
  std::string value(reinterpret_cast<char const *>(v));
  return value;
}

// Extract attribute value from xml-node based on attribute name
void* search_default_attrib_key(xmlNodePtr n) {
  string key, value;
  //check if the node has a name-attribute
  if (xmlHasProp(n, (const xmlChar *)"name") && xmlHasProp(n, (const xmlChar *)"value")) {
    key = getStringFromProp(n, "name");
    value = getStringFromProp(n, "value");
  }
  else {
    return 0;
  }
  // Handle attributes with uint64_t values
  if (!key.compare("CATcos") || !key.compare("CATL3mask")) {
    return new long long (std::strtoull((const char *)value.c_str(), NULL, 16));
  }
  // Handle attributes with long long values
  else if (!key.compare("mig_size")) {
    return new long long (std::strtoull((const char *)value.c_str(), NULL, 10));
  }

  // Handle attributes with int values
  else if (!key.compare("Number_of_streaming_multiprocessors") ||
           !key.compare("Number_of_cores_in_GPU") ||
           !key.compare("Number_of_cores_per_SM") ||
           !key.compare("Bus_Width_bit")) {
    return (new int(std::stoi(value)));
  }

  // Handle attributes with double values
  else if (!key.compare("Clock_Frequency")) {
    return new double(std::stod(value));
  }

  // Handle attributes with float values
  else if (!key.compare("latency") || !key.compare("latency_min") ||
           !key.compare("latency_max")) {
    return new float(std::stof(value));
  }

  // Handle attributes with string values
  else if (!key.compare("CUDA_compute_capability") ||
           !key.compare("mig_uuid")) {
    return new std::string(value);
  }

  return NULL; // Attribute not found or not handled
}
// Search for custom complex attributes in xmlNode n and add them to Component c
//
// Complex attributes are attributes that have a value that is not a simple type
// but a more complex structure like a vector
int search_default_complex_attrib_key(xmlNodePtr n, Component *c) {
  string key;
  if (xmlHasProp(n, (const xmlChar *)"name")) {
    key = getStringFromProp(n, "name");
  }
  else {
    return 0;
  }
  // freq_history is a vector of tuples containing the timestamp and the
  // frequency
  if (!key.compare("freq_history")) {
    std::vector<std::tuple<long long, double>> val;
    xmlNodePtr attr = n->children;
    for (xmlNodePtr cur = attr->children; cur != NULL; cur = cur->next) {
      // skip text nodes
      if (cur->type == XML_TEXT_NODE)
        continue;

      // add value to vector

      string ts = getStringFromProp(cur, "timestamp");
      string freq = getStringFromProp(cur, "frequency");

      long long ts_ll = std::strtoll((const char *)ts.c_str(), NULL, 10);
      double freq_d = std::stod(freq);

      val.push_back(std::make_tuple(ts_ll, freq_d));
    }
    void *value = (void *)&val;
    c->attrib[key] = value;
    return 1;
  } else if (!key.compare("GPU_Clock_Rate")) {
    // GPU_Clock_Rate is a vector of tuples containing the frequency and the
    // unit
    std::vector<std::tuple<double, std::string>> val;

    xmlNodePtr attr = n->children;
    xmlNodePtr cur = attr->children;

    string unit = getStringFromProp(cur, "unit");
    string freq = getStringFromProp(cur, "frequency");

    double freq_d = std::stod(freq);

    val.push_back(std::make_tuple(freq_d, unit));
    void *value = (void *)&val;
    c->attrib[key] = value;

    return 1;
  }

  return 0;
}

// Collect all attributes and add to Component c
//
// This function traverses the xmlNode n and extracts all attributes.
// The attributes are added to the Component c.
// If the custom functions are not null, they are called first. If they
// can not handle the attribute, the default functions are used.
int collect_attrib(xmlNodePtr n, Component *c) {
  void *attrib_value = NULL;
  // try custom attribute search function
  if (search_custom_attrib_key_fcn != NULL)
    attrib_value = search_custom_attrib_key_fcn(n);
  // if custom function could not handle attribute, try default
  if (attrib_value == NULL)
    attrib_value = search_default_attrib_key(n);
  // if attribute was handled, add it to Component
  if (attrib_value != NULL) {
    string key = getStringFromProp(n, "name");
    c->attrib[key] = attrib_value;
  }
  int ret = 0;
  // try custom complex attribute search function
  if (attrib_value == NULL && search_custom_complex_attrib_key_fcn != NULL)
     ret = search_custom_complex_attrib_key_fcn(n, c);
  // if custom function could not handle attribute, try default
  if (attrib_value == NULL && ret == 0)
    return search_default_complex_attrib_key(n, c);

  return 1;
}

// Create ComponentSubtree from xmlNodes
//
// This function creates a ComponentSubtree from the xmlNode n by creating
// a Component and then recursively calling itself for all children of n.
Component *CreateComponentSubtree(xmlNodePtr n, string type) {
  Component *c = NULL;

  std::string name = getStringFromProp(n, "name");
  int id = std::stoi(getStringFromProp(n, "id"));
  std::string addr = getStringFromProp(n, "addr");

  // Check the type of Component and create the corresponding Component
  if (type.compare("Memory") == 0) {
    long long size = 0;
    bool is_volatile = false;
    if (xmlHasProp(n, (const xmlChar *)"size")) {
      string value = getStringFromProp(n, "size");
      size = std::stoll(value);
    }
    if (xmlHasProp(n, (const xmlChar *)"is_volatile")) {
      string value = getStringFromProp(n, "is_volatile");
      is_volatile = (std::string(value) == "true");
    }
    c = new Memory(NULL, id, name, size, is_volatile);
  }
  if (type.compare("Storage") == 0) {
    // Same as with memory
    long long size = 0;
    if (xmlHasProp(n, (const xmlChar *)"size")) {
      string value = getStringFromProp(n, "size");
      size = std::stoll(value);
    }
    c = new Storage();
    ((Storage *)c)->SetSize(size);
  }
  if (type.compare("Chip") == 0) {

    c = new Chip(id);
    // check for vendor and model
    if (xmlHasProp(n, (const xmlChar *)"vendor")) {
      string value = getStringFromProp(n, "vendor");
      ((Chip *)c)->SetVendor(value);
    }
    if (xmlHasProp(n, (const xmlChar *)"model")) {
      string model = getStringFromProp(n, "model");
      ((Chip *)c)->SetModel(model);
    }
  }
  if (type.compare("Cache") == 0) {

    c = new Cache(id);
    if (xmlHasProp(n, (const xmlChar *)"cache_level")) {
      string value = getStringFromProp(n, "cache_level");
      ((Cache *)c)->SetCacheLevel(std::stoi(value));
    }
    if (xmlHasProp(n, (const xmlChar *)"cache_size")) {
      string value = getStringFromProp(n, "cache_size");
      ((Cache *)c)->SetCacheSize(std::stoi(value));
    }
    if (xmlHasProp(n, (const xmlChar *)"cache_associativity_ways")) {
      string value = getStringFromProp(n, "cache_associativity_ways");
      ((Cache *)c)->SetCacheAssociativityWays(std::stoi(value));
    }
    if (xmlHasProp(n, (const xmlChar *)"cache_line_size")) {
      string value = getStringFromProp(n, "cache_line_size");
      ((Cache *)c)->SetCacheLineSize(std::stoi(value));
    }
  }
  if (type.compare("Subdivision") == 0) {
    c = new Subdivision(id);
    int sd_type = std::stoi(getStringFromProp(n, "type"));
    
    ((Subdivision *)c)->SetSubdivisionType(sd_type);
  }
  if (type.compare("NUMA") == 0) {
    c = new Numa(id);
    if (xmlHasProp(n, (const xmlChar *)"size")) {
      string value = getStringFromProp(n, "size");
      ((Numa *)c)->SetSize(std::stoll(value));
    }
  }
  if (type.compare("None") == 0) {
    c = new Component((id));
  }
  if (type.compare("HW_thread") == 0) {
    c = new Thread(id);
  }
  if (type.compare("Core") == 0) {
    c = new Core(id);
  }
  if (type.compare("Node") == 0) {
    c = new Node(id);
  }
  if (type.compare("Topology") == 0) {
    c = new Topology();
  }

  // Recursively traverse all children of n and create Components
  for (xmlNodePtr cur = n->children; cur != NULL; cur = cur->next) {
    std::string type(reinterpret_cast<const char *>(cur->name));

    // Skip Text nodes
    if (std::string("text").compare(type) == 0)
      continue;
    // Check if cur is Attribute-Node
    if (type.compare("Attribute") == 0) {
      collect_attrib(cur, c);
    } else {
      Component *child = CreateComponentSubtree(cur, type);
      if (child != NULL) {
        c->InsertChild(child);
      }
    }
  }
  // Add the Component to the hashmap
  addr_to_component[addr] = c;
  return c;
}

// Create DataPath objects from xmlNode dpNode and add them to the
// corresponding Components
int CreateDataPaths(xmlNodePtr dpNode) {

  for (xmlNodePtr cur = dpNode->children; cur != NULL; cur = cur->next) {

    // skip non-element nodes
    if (cur->type != XML_ELEMENT_NODE)
      continue;
    string src = getStringFromProp(cur, "source");
    string trg = getStringFromProp(cur, "target");
    int oriented = std::stoi(getStringFromProp(cur, "oriented"));
    int dp_type = std::stoi(getStringFromProp(cur, "dp_type"));
    double bw = std::stod(getStringFromProp(cur, "bw"));
    double latency = std::stod(getStringFromProp(cur, "latency"));

    // get source and target Components from hashmap
    Component *src_c = addr_to_component[src];
    Component *trg_c = addr_to_component[trg];
    // Datapath constructor adds dp to Component-objects. Also handles
    // bidirectional relations
    new DataPath(src_c, trg_c, oriented, dp_type, bw, latency);
  }
  return 1;
}

Component *importFromXml(
    string path,
    std::function<void*(xmlNodePtr)> _search_custom_attrib_key_fcn,
    std::function<int(xmlNodePtr, Component *)>
        _search_custom_complex_attrib_key_fcn) {

  search_custom_attrib_key_fcn = _search_custom_attrib_key_fcn;
  search_custom_complex_attrib_key_fcn = _search_custom_complex_attrib_key_fcn;

  xmlInitParser();
  xmlDocPtr doc = xmlReadFile(path.c_str(), NULL, 0);
  xmlNodePtr sys_sage_root = xmlDocGetRootElement(doc);

  xmlNodePtr root = sys_sage_root->children;
  xmlNodePtr r = root->next;

  Component *c = CreateComponentSubtree(r->children->next, "Topology");

  xmlNodePtr dp_root = r->next->next;

  CreateDataPaths(dp_root);

  return c;
}


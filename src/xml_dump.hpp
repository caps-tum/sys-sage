#ifndef XML_DUMP
#define XML_DUMP

#include <functional>

#include "Topology.hpp"
#include "DataPath.hpp"

int exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> search_custom_attrib_key_fcn = NULL, std::function<int(string,void*,xmlNodePtr)> search_custom_complex_attrib_key_fcn = NULL);
int search_default_attrib_key(string key, void* value, string* ret_value_str);

int print_attrib(map<string,void*> attrib, xmlNodePtr n);
#endif

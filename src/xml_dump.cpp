#include <sstream>

#include "xml_dump.hpp"
#include <libxml/parser.h>

std::function<int(string,void*,string*)> search_custom_attrib_key_fcn = NULL;
std::function<int(string,void*,xmlNodePtr)> search_custom_complex_attrib_key_fcn = NULL;

//for a specific key, return the value as a string to be printed in the xml
int search_default_attrib_key(string key, void* value, string* ret_value_str)
{
    if(!key.compare("CATcos") || !key.compare("CATL3mask"))
    {
        *ret_value_str=std::to_string(*(uint64_t*)value);
        return 1;
    }
    else if(!key.compare("CUDA_compute_capability"))
    {
        *ret_value_str=*(string*)value;
        return 1;
    }
    else if(!key.compare("Number_of_streaming_multiprocessors") || !key.compare("Number_of_cores_in_GPU") || !key.compare("Number_of_cores_per_SM")  || !key.compare("Bus_Width_bit"))
    {
        *ret_value_str=std::to_string(*(int*)value);
        return 1;
    }
    else if(!key.compare("Clock_Frequency"))
    {
        *ret_value_str=std::to_string(*(double*)value);
        return 1;
    }

    return 0;
}

int search_default_complex_attrib_key(string key, void* value, xmlNodePtr n)
{
    //value: std::vector<std::tuple<long long,double>>*
    if(!key.compare("freq_history"))
    {
        std::vector<std::tuple<long long,double>>* val = (std::vector<std::tuple<long long,double>>*)value;

        xmlNodePtr attrib_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
        xmlNewProp(attrib_node, (const unsigned char *)"name", (const unsigned char *)key.c_str());
        xmlAddChild(n, attrib_node);
        for(auto [ ts,freq ] : *val)
        {
            xmlNodePtr attrib = xmlNewNode(NULL, (const unsigned char *)key.c_str());
            xmlNewProp(attrib, (const unsigned char *)"timestamp", (const unsigned char *)std::to_string(ts).c_str());
            xmlNewProp(attrib, (const unsigned char *)"frequency", (const unsigned char *)std::to_string(freq).c_str());
            xmlNewProp(attrib, (const unsigned char *)"unit", (const unsigned char *)"MHz");
            xmlAddChild(attrib_node, attrib);
        }
        return 1;
    }
    //value: std::tuple<double, std::string>
    else if(!key.compare("GPU_Clock_Rate"))
    {
        xmlNodePtr attrib_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
        xmlNewProp(attrib_node, (const unsigned char *)"name", (const unsigned char *)key.c_str());
        xmlAddChild(n, attrib_node);

        auto [ freq, unit ] = *(std::tuple<double, std::string>*)value;
        xmlNodePtr attrib = xmlNewNode(NULL, (const unsigned char *)key.c_str());
        xmlNewProp(attrib, (const unsigned char *)"frequency", (const unsigned char *)std::to_string(freq).c_str());
        xmlNewProp(attrib, (const unsigned char *)"unit", (const unsigned char *)unit.c_str());
        xmlAddChild(attrib_node, attrib);
        return 1;
    }

    return 0;
}

int print_attrib(map<string,void*> attrib, xmlNodePtr n)
{
    string attrib_value;
    for (auto const& [key, val] : attrib){
        int ret = 0;
        if(search_custom_attrib_key_fcn != NULL)
            ret=search_custom_attrib_key_fcn(key,val,&attrib_value);
        if(ret==0)
            ret = search_default_attrib_key(key,val,&attrib_value);

        if(ret==1)//attrib found
        {
            xmlNodePtr attrib_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
            xmlNewProp(attrib_node, (const unsigned char *)"name", (const unsigned char *)key.c_str());
            xmlNewProp(attrib_node, (const unsigned char *)"value", (const unsigned char *)attrib_value.c_str());
            xmlAddChild(n, attrib_node);
            continue;
        }

        if(ret == 0 && search_custom_complex_attrib_key_fcn != NULL) //try looking in search_custom_complex_attrib_key
            ret=search_custom_complex_attrib_key_fcn(key,val,n);
        if(ret==0)
            ret = search_default_complex_attrib_key(key,val,n);
    }

    return 1;
}

xmlNodePtr Memory::CreateXmlSubtree()
{
    xmlNodePtr n = Component::CreateXmlSubtree();
    if(size > 0)
        xmlNewProp(n, (const unsigned char *)"size", (const unsigned char *)(std::to_string(size)).c_str());
    xmlNewProp(n, (const unsigned char *)"is_volatile", (const unsigned char *)(std::to_string(is_volatile?1:0)).c_str());
    return n;
}
xmlNodePtr Storage::CreateXmlSubtree()
{
    xmlNodePtr n = Component::CreateXmlSubtree();
    if(size > 0)
        xmlNewProp(n, (const unsigned char *)"size", (const unsigned char *)(std::to_string(size)).c_str());
    return n;
}
xmlNodePtr Chip::CreateXmlSubtree()
{
    xmlNodePtr n = Component::CreateXmlSubtree();
    if(!vendor.empty())
        xmlNewProp(n, (const unsigned char *)"vendor", (const unsigned char *)(vendor.c_str()));
    if(!model.empty())
        xmlNewProp(n, (const unsigned char *)"model", (const unsigned char *)(model.c_str()));
    return n;
}
xmlNodePtr Cache::CreateXmlSubtree()
{
    xmlNodePtr n = Component::CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"cache_level", (const unsigned char *)cache_type.c_str());
    if(cache_size >= 0)
        xmlNewProp(n, (const unsigned char *)"cache_size", (const unsigned char *)(std::to_string(cache_size)).c_str());
    if(cache_associativity_ways >= 0)
        xmlNewProp(n, (const unsigned char *)"cache_associativity_ways", (const unsigned char *)(std::to_string(cache_associativity_ways)).c_str());
    if(cache_line_size >= 0)
        xmlNewProp(n, (const unsigned char *)"cache_line_size", (const unsigned char *)(std::to_string(cache_line_size)).c_str());
    return n;
}
xmlNodePtr Subdivision::CreateXmlSubtree()
{
    xmlNodePtr n = Component::CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"subdivision_type", (const unsigned char *)(std::to_string(type)).c_str());
    return n;
}
xmlNodePtr Numa::CreateXmlSubtree()
{
    xmlNodePtr n = Component::CreateXmlSubtree();
    if(size > 0)
        xmlNewProp(n, (const unsigned char *)"size", (const unsigned char *)(std::to_string(size)).c_str());
    return n;
}
xmlNodePtr Component::CreateXmlSubtree()
{
    xmlNodePtr n = xmlNewNode(NULL, (const unsigned char *)GetComponentTypeStr().c_str());
    xmlNewProp(n, (const unsigned char *)"id", (const unsigned char *)(std::to_string(id)).c_str());
    xmlNewProp(n, (const unsigned char *)"name", (const unsigned char *)name.c_str());
    if(count > 0)
        xmlNewProp(n, (const unsigned char *)"count", (const unsigned char *)(std::to_string(count)).c_str());
    std::ostringstream addr;
    addr << this;
    xmlNewProp(n, (const unsigned char *)"addr", (const unsigned char *)(addr.str().c_str()));

    print_attrib(attrib, n);

    for(Component * c : children)
    {
        xmlNodePtr child;
        switch (c->GetComponentType()) {
            case SYS_SAGE_COMPONENT_CACHE:
                child = ((Cache*)c)->CreateXmlSubtree();
                break;
            case SYS_SAGE_COMPONENT_SUBDIVISION:
                child = ((Subdivision*)c)->CreateXmlSubtree();
                break;
            case SYS_SAGE_COMPONENT_NUMA:
                child = ((Numa*)c)->CreateXmlSubtree();
                break;
            case SYS_SAGE_COMPONENT_CHIP:
                child = ((Chip*)c)->CreateXmlSubtree();
                break;
            case SYS_SAGE_COMPONENT_MEMORY:
                child = ((Memory*)c)->CreateXmlSubtree();
                break;
            case SYS_SAGE_COMPONENT_STORAGE:
                child = ((Storage*)c)->CreateXmlSubtree();
                break;
            case SYS_SAGE_COMPONENT_NONE:
            case SYS_SAGE_COMPONENT_THREAD:
            case SYS_SAGE_COMPONENT_CORE:
            case SYS_SAGE_COMPONENT_NODE:
            case SYS_SAGE_COMPONENT_TOPOLOGY:
            default:
                child = c->CreateXmlSubtree();
                break;
        };

        xmlAddChild(n, child);
    }


    return n;
}

int exportToXml(Component* root, string path, std::function<int(string,void*,string*)> _search_custom_attrib_key_fcn, std::function<int(string,void*,xmlNodePtr)> _search_custom_complex_attrib_key_fcn)
{
    search_custom_attrib_key_fcn=_search_custom_attrib_key_fcn;
    search_custom_complex_attrib_key_fcn=_search_custom_complex_attrib_key_fcn;

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

    xmlNodePtr sys_sage_root = xmlNewNode(NULL, BAD_CAST "sys-sage");
    xmlDocSetRootElement(doc, sys_sage_root);
    xmlNodePtr components_root = xmlNewNode(NULL, BAD_CAST "components");
    xmlAddChild(sys_sage_root, components_root);
    xmlNodePtr data_paths_root = xmlNewNode(NULL, BAD_CAST "data-paths");
    xmlAddChild(sys_sage_root, data_paths_root);

    //build a tree for Components
    xmlNodePtr n = root->CreateXmlSubtree();
    xmlAddChild(components_root, n);

    //scan all Components for their DataPaths
    vector<Component*> components;
    root->GetSubtreeNodeList(&components);
    std::cout << "Number of components to export: " << components.size() << std::endl;
    for(Component* cPtr : components)
    {
        vector<DataPath*>* dpList = cPtr->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
        vector<DataPath*> printed_dp;

        for(DataPath* dpPtr : *dpList)
        {
            //check if previously processed
            if (std::find(printed_dp.begin(), printed_dp.end(), dpPtr) == printed_dp.end())
            {
                xmlNodePtr dp_n = xmlNewNode(NULL, BAD_CAST "datapath");
                std::ostringstream src_addr;
                src_addr << dpPtr->GetSource();
                std::ostringstream target_addr;
                target_addr << dpPtr->GetTarget();
                xmlNewProp(dp_n, (const unsigned char *)"source", (const unsigned char *)(src_addr.str().c_str()));
                xmlNewProp(dp_n, (const unsigned char *)"target", (const unsigned char *)(target_addr.str().c_str()));
                xmlNewProp(dp_n, (const unsigned char *)"oriented", (const unsigned char *)(std::to_string(dpPtr->GetOriented())).c_str());
                xmlNewProp(dp_n, (const unsigned char *)"dp_type", (const unsigned char *)(std::to_string(dpPtr->GetDpType())).c_str());
                xmlNewProp(dp_n, (const unsigned char *)"bw", (const unsigned char *)(std::to_string(dpPtr->GetBw())).c_str());
                xmlNewProp(dp_n, (const unsigned char *)"latency", (const unsigned char *)(std::to_string(dpPtr->GetLatency())).c_str());
                xmlAddChild(data_paths_root, dp_n);

                print_attrib(dpPtr->attrib, dp_n);

                printed_dp.push_back(dpPtr);
            }
        }
    }

    xmlSaveFormatFileEnc(path=="" ? "-" : path.c_str(), doc, "UTF-8", 1);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

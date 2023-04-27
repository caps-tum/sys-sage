
#include "gpu-topo.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <tuple>
#include <string>


int parseGpuTopo(Component* parent, string dataSourcePath, int gpuId, string delim)
{
    if(parent == NULL){
        std::cerr << "parseGpuTopo: parent is null" << std::endl;
        return 1;
    }
    Chip * gpu = new Chip(parent, gpuId, "GPU", SYS_SAGE_CHIP_TYPE_GPU);

    return parseGpuTopo(gpu, dataSourcePath, delim);
}

int parseGpuTopo(Chip* gpu, string dataSourcePath, string delim)
{
    GpuTopo gpuT(gpu, dataSourcePath, delim);
    int ret = gpuT.ParseBenchmarkData();
    return ret;

}

GpuTopo::GpuTopo(Chip* gpu, string dataSourcePath, string delim) : dataSourcePath(dataSourcePath), delim(delim), root(gpu), Memory_Clock_Frequency(-1), Memory_Bus_Width(-1)  { }

int GpuTopo::ReadBenchmarkFile()
{
    std::ifstream file(dataSourcePath);
    if (!file.good()){
        std::cerr << "parseGpuTopo: could not open data source output file " << dataSourcePath << std::endl;
        return 1;
    }

    std::string line = "";
    while (getline(file, line))
    {
        std::vector<std::string> vec;
        size_t pos = 0;
        bool cont = true;
        string s;
        while (cont) {
            cont = ((pos = line.find(delim)) != std::string::npos);
            if(cont)
                s = line.substr(0, pos); // trim whitespaces
            else
                s = line.substr(0, line.length());
            trim(s);
            s.erase(std::remove(s.begin(), s.end(), '\"'), s.end());    //remove "" where present
            //std::cout << "adding " << s << std::endl;
            vec.push_back(s);
            line.erase(0, pos + delim.length());
        }
        if(!vec.empty()) {
            benchmarkData.insert({vec[0], vec});
        }
    }
    return 0;
}

int GpuTopo::ParseBenchmarkData()
{
    int ret = ReadBenchmarkFile();
    if(ret != 0)
        return ret;

    if(benchmarkData.find("GPU_INFORMATION") == benchmarkData.end()){
        cerr << "parseGpuTopo: Could not find GPU_INFORMATION in file " << dataSourcePath << endl;
        return 1;
    } else {
        if((ret=parseGPU_INFORMATION()) != 0){
            cerr << "parseGpuTopo: parseGPU_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("COMPUTE_RESOURCE_INFORMATION") == benchmarkData.end()){
        cerr << "parseGpuTopo: Could not find COMPUTE_RESOURCE_INFORMATION in file " << dataSourcePath << endl;
        return 1;
    } else {
        if((ret=parseCOMPUTE_RESOURCE_INFORMATION()) != 0){
            cerr << "parseGpuTopo: parseCOMPUTE_RESOURCE_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("REGISTER_INFORMATION") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find REGISTER_INFORMATION in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseREGISTER_INFORMATION()) != 0){
            cerr << "parseGpuTopo: parseREGISTER_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("ADDITIONAL_INFORMATION") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find ADDITIONAL_INFORMATION in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseADDITIONAL_INFORMATION()) != 0){
            cerr << "parseGpuTopo: parseADDITIONAL_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("MAIN_MEMORY") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find MAIN_MEMORY in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseMAIN_MEMORY()) != 0){
            cerr << "parseGpuTopo: parseMAIN_MEMORY failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("L2_DATA_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find L2_DATA_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("L2_DATA_CACHE", "L2")) != 0){
            cerr << "parseGpuTopo: parseCaches on L2_DATA_CACHE failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("L1_DATA_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find L1_DATA_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("L1_DATA_CACHE", "L1")) != 0){
            cerr << "parseGpuTopo: parseCaches failed on L1_DATA_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("SHARED_MEMORY") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find SHARED_MEMORY in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("SHARED_MEMORY", "Shared_Memory")) != 0){
            cerr << "parseGpuTopo: parseCaches failed on SHARED_MEMORY when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("TEXTURE_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find TEXTURE_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("TEXTURE_CACHE", "Texture")) != 0){
            cerr << "parseGpuTopo: parseCaches failed on TEXTURE_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("READ-ONLY_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find READ-ONLY_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("READ-ONLY_CACHE", "ReadOnly")) != 0){
            cerr << "parseGpuTopo: parseCaches failed on READ-ONLY_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("CONST_L1_5_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find CONST_L1_5_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("CONST_L1_5_CACHE", "Constant_L1.5")) != 0){
            cerr << "parseGpuTopo: parseCaches failed on CONST_L1_5_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("CONSTANT_L1_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseGpuTopo: Could not find CONSTANT_L1_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("CONSTANT_L1_CACHE", "Constant_L1")) != 0){
            cerr << "parseGpuTopo: parseCaches failed on CONSTANT_L1_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    ret = root->CheckComponentTreeConsistency();
    return ret;
}

int GpuTopo::parseGPU_INFORMATION()
{
    vector<string> data = benchmarkData["GPU_INFORMATION"];
    data.erase(data.begin());

    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i] == "GPU_vendor")
        {
            if(i>=data.size()-1){
                cerr << "parseGPU_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            root->SetVendor(data[i+1]);
            i++;
        }
        else if(data[i] == "GPU_name")
        {
            if(i>=data.size()-1){
                cerr << "parseGPU_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            root->SetModel(data[i+1]);
            i++;
        }
    }
    return 0;
}

int GpuTopo::parseCOMPUTE_RESOURCE_INFORMATION()
{
    vector<string> data = benchmarkData["COMPUTE_RESOURCE_INFORMATION"];
    data.erase(data.begin());

    for(size_t i = 0; i<data.size(); i++)
    {
        //cout << i << " " << data[i] << std::endl;
        if(data[i]== "CUDA_compute_capability")
        {
            if(i>=data.size()-1){
                cerr << "parseCOMPUTE_RESOURCE_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            string * val = new string(data[i+1]);
            root->attrib.insert({data[i], (void*)val});
            i++;
        }
        else if(data[i]== "Number_of_streaming_multiprocessors" ||
            data[i]== "Number_of_cores_in_GPU" ||
            data[i]== "Number_of_cores_per_SM")
        {
            if(i>=data.size()-1){
                cerr << "parseCOMPUTE_RESOURCE_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            int * val = new int(std::stoi(data[i+1]));
            root->attrib.insert({data[i], (void*)val});

            i++;
        }
    }

    for(int i = 0; i < (*(int*)root->attrib["Number_of_streaming_multiprocessors"]); i++)
    {
        //cout << "adding SM " << i << std::endl;
        Subdivision * sm = new Subdivision(root, i, "SM (Streaming Multiprocessor)");
        sm->SetSubdivisionType(SYS_SAGE_SUBDIVISION_TYPE_GPU_SM);
        for(int j = 0; j<(*(int*)root->attrib["Number_of_cores_per_SM"]); j++)
        {
            new Thread(sm, j, "GPU Core");
        }
    }
    return 0;
}

int GpuTopo::parseREGISTER_INFORMATION()
{
    //TODO
    return 0;
}
int GpuTopo::parseADDITIONAL_INFORMATION()
{
    vector<string> data = benchmarkData["ADDITIONAL_INFORMATION"];
    data.erase(data.begin());

    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i]== "Memory_Clock_Frequency")
        {
            if(i>=data.size()-2){
                cerr << "parseREGISTER_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            Memory_Clock_Frequency = stod(data[i+1]);
            string unit = data[i+2];
            if(unit == "KHz")
                Memory_Clock_Frequency *= 1024;
            else if(unit == "MHz")
                Memory_Clock_Frequency *= 1024*1024;
            else if(unit == "GHz")
                Memory_Clock_Frequency *= 1024*1024*1024;
            i+=2;
        }
        else if(data[i]== "Memory_Bus_Width")
        {
            if(i>=data.size()-2){
                cerr << "parseREGISTER_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            Memory_Bus_Width = stod(data[i+1]);
            string unit = data[i+2];
            if(unit == "KHz")
                Memory_Bus_Width *= 1024;
            else if(unit == "MHz")
                Memory_Bus_Width *= 1024*1024;
            else if(unit == "GHz")
                Memory_Bus_Width *= 1024*1024*1024;
            i+=2;
        }
        else if(data[i]== "GPU_Clock_Rate")
        {
            if(i>=data.size()-2){
                cerr << "parseREGISTER_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            std::tuple<double, std::string>* val = new std::tuple<double, std::string>(stod(data[i+1]), data[i+2]);
            root->attrib.insert({data[i], (void*)val});
            i+=2;
        }
    }
    return 0;
}
int GpuTopo::parseMAIN_MEMORY()
{
    vector<string> data = benchmarkData["MAIN_MEMORY"];
    data.erase(data.begin());

    int shared_on = -1; //0=GPU, 1=SM
    double size = -1;
    double latency = -1;
    //parse_args
    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i]== "Size")
        {
            if(i>=data.size()-3){
                cerr << "parseMAIN_MEMORY: \"" << data[i] << "\" is supposed to be followed by 3 additional values." << endl;
                return 1;
            }
            size = stod(data[i+1]);
            string unit = data[i+2];
            if(unit == "KiB")
                size *= 1024;
            else if(unit == "MiB")
                size *= 1024*1024;
            else if(unit == "GiB")
                size *= 1024*1024*1024;
            i+=3;
        }
        else if(data[i]== "Load_Latency")
        {
            if(i>=data.size()-2){
                cerr << "parseMAIN_MEMORY: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            if(data[i+2] == "cycles")
            {
                latency = stod(data[i+1]);
            }
            i+=2;
        }
        else if(data[i]== "Shared_On")
        {
            if(i>=data.size()-1){
                cerr << "parseMAIN_MEMORY: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            if(data[i+1] == "GPU-level")
                shared_on = 0;
            else if(data[i+1] == "SM-level")
                shared_on = 1;
            else
            {
                cerr << "parseMAIN_MEMORY: \"" << data[i] << "\" is supposed to be GPU-level or SM-level." << endl;
                return 1;
            }
            i+=1;
        }
    }
    if(shared_on == 0)
    {
        Memory * mem = new Memory(root, "GPU main memory");
        if(size != -1)
            mem->SetSize((long long)size);

        if(Memory_Clock_Frequency > -1){
            double * mfreq = new double(Memory_Clock_Frequency);
            mem->attrib.insert({"Clock_Frequency", (void*)mfreq});
        }
        if(Memory_Bus_Width > -1){
            int * busW = new int(Memory_Bus_Width);
            mem->attrib.insert({"Bus_Width_bit", (void*)busW});
        }

        //make SMs as memory's children and inserd DP with latency
        vector<Component*> children_copy;
        for(Component* child : *(root->GetChildren())){
            children_copy.push_back(child);
        }
        for(Component * sm : children_copy)
        {
            if(sm->GetComponentType() == SYS_SAGE_COMPONENT_SUBDIVISION && ((Subdivision*)sm)->GetSubdivisionType() == SYS_SAGE_SUBDIVISION_TYPE_GPU_SM)
            {
                root->RemoveChild(sm);
                mem->InsertChild(sm);
                sm->SetParent(mem);

                if(latency != -1)
                    for(Component * c : *(sm->GetChildren()))
                        if(c->GetComponentType() == SYS_SAGE_COMPONENT_THREAD)
                            DataPath * d = new DataPath(mem, c, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL, 0, latency);
            }
        }
    }
    else if(shared_on == 1)
    {
        std::cerr << "GpuTopo::parseMAIN_MEMORY, shared_on == 1 --> this should not happen...if yes, the implementation needs to be extended." << std::endl;
        return 1;
    }

    return 0;
}

int GpuTopo::parseCaches(string header_name, string cache_type)
{
    vector<string> data = benchmarkData[header_name];
    data.erase(data.begin());

    //parse_args
    int shared_on = -1; //0=GPU, 1=SM
    int caches_per_sm = 1;
    double size = -1;
    int cache_line_size = -1;
    double latency = -1;
    int share_l1 = 0, share_texture = 0, share_ro = 0, share_constant = 0;
    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i]== "Size")
        {
            if(i>=data.size()-3){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            size = stod(data[i+1]);
            string unit = data[i+2];
            if(unit == "KiB")
                size *= 1024;
            else if(unit == "MiB")
                size *= 1024*1024;
            else if(unit == "GiB")
                size *= 1024*1024*1024;
            i+=3;
        }
        else if(data[i]== "Cache_Line_Size")
        {
            if(i>=data.size()-2){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            cache_line_size = stoi(data[i+1]);
            string unit = data[i+2];
            if(unit == "KiB")
                size *= 1024;
            else if(unit == "MiB")
                size *= 1024*1024;
            else if(unit == "GiB")
                size *= 1024*1024*1024;
            i+=3;
        }
        else if(data[i]== "Load_Latency")
        {
            if(i>=data.size()-2){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            if(data[i+2] == "cycles")
            {
                latency = stod(data[i+1]);
            }
            i+=2;
        }
        else if(data[i]== "Shared_On")
        {
            if(i>=data.size()-1){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            if(data[i+1] == "GPU-level")
                shared_on = 0;
            else if(data[i+1] == "SM-level")
                shared_on = 1;
            else
            {
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be GPU-level or SM-level." << endl;
                return 1;
            }
            i+=1;
        }
        else if(data[i]== "Caches_Per_SM")
        {
            if(i>=data.size()-1){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            caches_per_sm = stoi(data[i+1]);
        }
        else if(data[i]== "Share_Cache_With_L1_Data")
        {
            if(i>=data.size()-1){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            share_l1 = stoi(data[i+1]);
        }
        else if(data[i]== "Share_Cache_With_Texture")
        {
            if(i>=data.size()-1){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            share_texture = stoi(data[i+1]);
        }
        else if(data[i]== "Share_Cache_With_Read-Only")
        {
            if(i>=data.size()-1){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            share_ro = stoi(data[i+1]);
        }
        else if(data[i]== "Share_Cache_With_ConstantL1")
        {
            if(i>=data.size()-1){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            share_constant = stoi(data[i+1]);
        }
    }

    if(cache_type == "L2")
    {
        if(shared_on == 0)
            L2_shared_on_gpu = true;
        else
            L2_shared_on_gpu = false;
    }

    //if caches are shared, add process only the first one and add names of the others
    if(cache_type == "L1")
    {
        if(share_texture == 1)
            cache_type.append("+Texture");
        if(share_ro == 1)
            cache_type.append("+ReadOnly");
        if(share_constant == 1)
            cache_type.append("+Constant_L1");
    }
    else if(cache_type == "Texture")
    {
        if(share_l1 == 1)
            return 0;//already added in L1
        if(share_ro == 1)
            cache_type.append("+ReadOnly");
        if(share_constant == 1)
            cache_type.append("+Constant_L1");
    }
    else if(cache_type == "ReadOnly")
    {
        if(share_l1 == 1 || share_texture == 1)
            return 0;//already added in L1 or RO
        if(share_constant == 1)
            cache_type.append("+Constant_L1");
    }
    else if(cache_type == "Constant_L1")
    {
        if(share_l1 == 1  || share_texture == 1 || share_ro == 1)
            return 0;//already added in L1 or texture or RO
    }

    Component* parent = root;
    if(shared_on == 0)
    { //shared on GPU level, place under main memory or L2(if not L2)
        Component * mem = root->GetChildByType(SYS_SAGE_COMPONENT_MEMORY);
        if(mem != NULL)
        {
            parent = mem;
            if(cache_type != "L2")
            {
                Component * l2 = mem->GetChildByType(SYS_SAGE_COMPONENT_CACHE);
                if(((Cache*)l2)->GetCacheName() == "L2")
                    parent = l2;
            }
        }
        Cache * cache = new Cache(parent, 0, cache_type);
        if(size != -1)
            cache->SetCacheSize(size);
        if(cache_line_size != -1)
            cache->SetCacheLineSize(cache_line_size);

        vector<Component*> children_copy;
        for(Component* child : *(parent->GetChildren())){
            children_copy.push_back(child);
        }
        for(Component * sm : children_copy)
        {
            if(sm->GetComponentType() == SYS_SAGE_COMPONENT_SUBDIVISION && ((Subdivision*)sm)->GetSubdivisionType() == SYS_SAGE_SUBDIVISION_TYPE_GPU_SM)
            {
                parent->RemoveChild(sm);
                cache->InsertChild(sm);
                sm->SetParent(cache);

                if(latency != -1)
                    for(Component * c : *(sm->GetChildren()))
                        if(c->GetComponentType() == SYS_SAGE_COMPONENT_THREAD)
                            DataPath * d = new DataPath(cache, c, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL, 0, latency);
            }
        }
    }
    else if(shared_on == 1) //shared on SM
    {
        vector<Component*> parents;
        root->FindAllSubcomponentsByType(&parents, SYS_SAGE_COMPONENT_SUBDIVISION);
        for(Component * parent : parents)
        {
            //if L2 is not shared on GPU, it will be the parent
            if(cache_type != "L2" && !L2_shared_on_gpu)
            {
                vector<Component*> caches = parent->GetAllChildrenByType(SYS_SAGE_COMPONENT_CACHE);
                for(Component * cache: caches){
                    if(((Cache*)cache)->GetCacheName() == "L2"){
                        parent = cache;
                        break;
                    }
                }
            }
            //constant L1 is child of constant L1.5
            if(cache_type == "Constant_L1")
            {
                vector<Component*> caches = parent->GetAllChildrenByType(SYS_SAGE_COMPONENT_CACHE);
                for(Component * cache: caches){
                    if(((Cache*)cache)->GetCacheName() == "Constant_L1.5"){
                        parent = cache;
                        break;
                    }
                }
            }

            for(int i=0; i<caches_per_sm; i++)
            {
                Cache * cache = new Cache(parent, i, cache_type);
                if(size != -1)
                    cache->SetCacheSize(size);
                if(cache_line_size != -1)
                    cache->SetCacheLineSize(cache_line_size);

                int cores_per_cache = (*(int*)root->attrib["Number_of_cores_per_SM"])/caches_per_sm;

                //insert DP with latency
                vector<Component*> children_copy;
                for(Component* child : *(parent->GetChildren())){
                    children_copy.push_back(child);
                }
                for(Component * child : children_copy)
                {
                    if(child->GetComponentType() == SYS_SAGE_COMPONENT_THREAD)
                    {
                        //if multiple caches per SM, move 1/n-th of threads (by their ID) under each cache
                        int core_id = child->GetId();
                        if(core_id >= cores_per_cache*(i) && core_id < cores_per_cache*(i+1))
                        {
                            //for L1 and L2, move cores as children
                            if(cache_type.find("L1") != std::string::npos || cache_type == "L2")
                            {
                                parent->RemoveChild(child);
                                cache->InsertChild(child);
                                child->SetParent(cache);
                            }

                            if(latency != -1)
                                DataPath * d = new DataPath(cache, child, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL, 0, latency);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
void trimRight( std::string& str, const std::string& trimChars)
{
   std::string::size_type pos = str.find_last_not_of( trimChars );
   str.erase( pos + 1 );
}

void trimLeft( std::string& str, const std::string& trimChars)
{
   std::string::size_type pos = str.find_first_not_of( trimChars );
   str.erase( 0, pos );
}

void trim( std::string& str, const std::string& trimChars)
{
   trimRight( str, trimChars );
   trimLeft( str, trimChars );
}

#include "mt4g.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <tuple>
#include <string>
#include <algorithm>

using namespace sys_sage;

using std::cout;
using std::cerr;
using std::endl;

class Mt4gParser
{
public:
    Mt4gParser(Chip* gpu, std::string dataSourcePath, std::string delim = ";");

    int ParseBenchmarkData();
private:
    int ReadBenchmarkFile();
    std::map<std::string,std::vector<std::string> > benchmarkData;
    std::string dataSourcePath;
    std::string delim;
    Chip* root;
    const bool latency_in_cycles;
    bool L2_shared_on_gpu;
    double Memory_Clock_Frequency;
    int Memory_Bus_Width;

    int parseGPU_INFORMATION();
    int parseCOMPUTE_RESOURCE_INFORMATION();
    int parseREGISTER_INFORMATION();
    int parseADDITIONAL_INFORMATION();
    int parseMemory(std::string header_name, std::string memory_name);
    int parseCaches(std::string header_name, std::string cache_type);
};

const std::string whiteSpaces( " \f\n\r\t\v" );
void trimRight( std::string& str,const std::string& trimChars = whiteSpaces );
void trimLeft( std::string& str,const std::string& trimChars = whiteSpaces );
void trim( std::string& str, const std::string& trimChars = whiteSpaces );

int sys_sage::ParseMt4g_v0_1(Component* parent, const std::string &path, int gpuId, const std::string delim)
{
    if(parent == NULL){
        std::cerr << "parseMt4gTopo: parent is null" << std::endl;
        return 1;
    }
    Chip * gpu = new Chip(parent, gpuId, "GPU", sys_sage::ChipType::Gpu);

    return ParseMt4g_v0_1(gpu, path, delim);
}

int sys_sage::ParseMt4g_v0_1(Chip* gpu, const std::string &path, const std::string delim)
{
    Mt4gParser gpuT(gpu, path, delim);
    int ret = gpuT.ParseBenchmarkData();
    return ret;
}

Mt4gParser::Mt4gParser(Chip* gpu, std::string dataSourcePath, std::string delim) : dataSourcePath(dataSourcePath), delim(delim), root(gpu), latency_in_cycles(true), Memory_Clock_Frequency(-1), Memory_Bus_Width(-1) { }

int Mt4gParser::ReadBenchmarkFile()
{
    std::ifstream file(dataSourcePath);
    if (!file.good()){
        std::cerr << "parseMt4gTopo: could not open data source output file " << dataSourcePath << std::endl;
        return 1;
    }

    std::string line = "";
    while (getline(file, line))
    {
        std::vector<std::string> vec;
        size_t pos = 0;
        bool cont = true;
        std::string s;
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

int Mt4gParser::ParseBenchmarkData()
{
    int ret = ReadBenchmarkFile();
    if(ret != 0)
        return ret;

    if(benchmarkData.find("GPU_INFORMATION") == benchmarkData.end()){
        cerr << "parseMt4gTopo: Could not find GPU_INFORMATION in file " << dataSourcePath << endl;
        return 1;
    } else {
        if((ret=parseGPU_INFORMATION()) != 0){
            cerr << "parseMt4gTopo: parseGPU_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("COMPUTE_RESOURCE_INFORMATION") == benchmarkData.end()){
        cerr << "parseMt4gTopo: Could not find COMPUTE_RESOURCE_INFORMATION in file " << dataSourcePath << endl;
        return 1;
    } else {
        if((ret=parseCOMPUTE_RESOURCE_INFORMATION()) != 0){
            cerr << "parseMt4gTopo: parseCOMPUTE_RESOURCE_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("REGISTER_INFORMATION") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find REGISTER_INFORMATION in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseREGISTER_INFORMATION()) != 0){
            cerr << "parseMt4gTopo: parseREGISTER_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("ADDITIONAL_INFORMATION") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find ADDITIONAL_INFORMATION in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseADDITIONAL_INFORMATION()) != 0){
            cerr << "parseMt4gTopo: parseADDITIONAL_INFORMATION failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("MAIN_MEMORY") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find MAIN_MEMORY in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseMemory("MAIN_MEMORY", "GPU Global memory")) != 0){
            cerr << "parseMt4gTopo: parseMAIN_MEMORY failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("L2_DATA_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find L2_DATA_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("L2_DATA_CACHE", "L2")) != 0){
            cerr << "parseMt4gTopo: parseCaches on L2_DATA_CACHE failed when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("L1_DATA_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find L1_DATA_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("L1_DATA_CACHE", "L1")) != 0){
            cerr << "parseMt4gTopo: parseCaches failed on L1_DATA_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("SHARED_MEMORY") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find SHARED_MEMORY in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseMemory("SHARED_MEMORY", "Shared memory")) != 0){
            cerr << "parseMt4gTopo: parseCaches failed on SHARED_MEMORY when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("TEXTURE_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find TEXTURE_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("TEXTURE_CACHE", "Texture")) != 0){
            cerr << "parseMt4gTopo: parseCaches failed on TEXTURE_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("READ-ONLY_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find READ-ONLY_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("READ-ONLY_CACHE", "ReadOnly")) != 0){
            cerr << "parseMt4gTopo: parseCaches failed on READ-ONLY_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("CONST_L1_5_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find CONST_L1_5_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("CONST_L1_5_CACHE", "Constant_L1.5")) != 0){
            cerr << "parseMt4gTopo: parseCaches failed on CONST_L1_5_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    if(benchmarkData.find("CONSTANT_L1_CACHE") == benchmarkData.end()){
        cerr << "WARNING: parseMt4gTopo: Could not find CONSTANT_L1_CACHE in file " << dataSourcePath <<". Will skip."<< endl;
    } else {
        if((ret=parseCaches("CONSTANT_L1_CACHE", "Constant_L1")) != 0){
            cerr << "parseMt4gTopo: parseCaches failed on CONSTANT_L1_CACHE when parsing " << dataSourcePath << endl;
            return ret;
        }
    }

    ret = root->CheckComponentTreeConsistency();
    return ret;
}

int Mt4gParser::parseGPU_INFORMATION()
{
    std::vector<std::string> data = benchmarkData["GPU_INFORMATION"];
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

int Mt4gParser::parseCOMPUTE_RESOURCE_INFORMATION()
{
    std::vector<std::string> data = benchmarkData["COMPUTE_RESOURCE_INFORMATION"];
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
            std::string * val = new std::string(data[i+1]);
            root->attrib.insert({data[i], reinterpret_cast<void*>(val)});
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
            root->attrib.insert({data[i], reinterpret_cast<void*>(val)});

            i++;
        }
    }

    for(int i = 0; i < *reinterpret_cast<int*>(root->attrib["Number_of_streaming_multiprocessors"]); i++)
    {
        //cout << "adding SM " << i << std::endl;
        Subdivision * sm = new Subdivision(root, i, "SM (Streaming Multiprocessor)");
        sm->SetSubdivisionType(sys_sage::SubdivisionType::GpuSM);
        for(int j = 0; j<*reinterpret_cast<int*>(root->attrib["Number_of_cores_per_SM"]); j++)
        {
            new Thread(sm, j, "GPU Core");
        }
    }
    return 0;
}

int Mt4gParser::parseREGISTER_INFORMATION()
{
    //TODO
    return 0;
}
int Mt4gParser::parseADDITIONAL_INFORMATION()
{
    std::vector<std::string> data = benchmarkData["ADDITIONAL_INFORMATION"];
    data.erase(data.begin());

    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i]== "Memory_Clock_Frequency")
        {
            if(i>=data.size()-2){
                cerr << "parseADDITIONAL_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            Memory_Clock_Frequency = stod(data[i+1]);
            std::string unit = data[i+2];
            if(unit == "KHz")
                Memory_Clock_Frequency *= 1000;
            else if(unit == "MHz")
                Memory_Clock_Frequency *= 1000*1000;
            else if(unit == "GHz")
                Memory_Clock_Frequency *= 1000*1000*1000;
            i+=2;
        }
        else if(data[i]== "Memory_Bus_Width")
        {
            if(i>=data.size()-2){
                cerr << "parseADDITIONAL_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            Memory_Bus_Width = std::stoi(data[i + 1]);
            i+=2;
        }
        else if(data[i]== "GPU_Clock_Rate")
        {
            if(i>=data.size()-2){
                cerr << "parseADDITIONAL_INFORMATION: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            double * val = new double(std::stod(data[i+1]));
            std::string unit = data[i+2];
            if(unit == "KHz")
                *val *= 1000;
            else if(unit == "MHz")
                *val *= 1000*1000;
            else if(unit == "GHz")
                *val *= 1000*1000*1000;
            root->attrib.insert({data[i], reinterpret_cast<void*>(val)});
            i+=2;
        }
    }
    return 0;
}
int Mt4gParser::parseMemory(std::string header_name, std::string memory_name)
{
    std::vector<std::string> data = benchmarkData[header_name];
    data.erase(data.begin());

    int shared_on = -1; //0=GPU, 1=SM
    double size = -1;
    double latency = -1;
    int ret;
    //parse_args
    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i]== "Size")
        {
            if(i>=data.size()-3){
                cerr << "parseMemory: \"" << data[i] << "\" is supposed to be followed by 3 additional values." << endl;
                return 1;
            }
            size = stod(data[i+1]);
            std::string unit = data[i+2];
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
                cerr << "parseMemory: \"" << data[i] << "\" is supposed to be followed by 2 additional values." << endl;
                return 1;
            }
            if((data[i+2] == "cycles" && latency_in_cycles) || (data[i+2] == "nanoseconds" && !latency_in_cycles))
            {
                latency = stod(data[i+1]);
            }
            i+=2;
        }
        else if(data[i]== "Shared_On")
        {
            if(i>=data.size()-1){
                cerr << "parseMemory: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            if(data[i+1] == "GPU-level")
                shared_on = 0;
            else if(data[i+1] == "SM-level")
                shared_on = 1;
            else{
                cerr << "parseMemory: \"" << data[i] << "\" is supposed to be GPU-level or SM-level." << endl;
                return 1;
            }
            i+=1;
        }
    }

    if(header_name == "MAIN_MEMORY")
    {
        if(shared_on != 0){
            std::cerr << "Mt4gParser::parseMemory, shared_on == 1 --> this should not happen...if yes, the implementation needs to be extended." << std::endl;
            return 1;
        }

        Memory * mem = new Memory(root, 0, memory_name, (long long)size);
        if(Memory_Clock_Frequency > -1){
            double * mfreq = new double(Memory_Clock_Frequency);
            mem->attrib.insert({"Clock_Frequency", reinterpret_cast<void*>(mfreq)});
        }
        if(Memory_Bus_Width > -1){
            int * busW = new int(Memory_Bus_Width);
            mem->attrib.insert({"Bus_Width", reinterpret_cast<void*>(busW)});
        }
          
        //make SMs as main memory's children and insert DP with latency
        std::vector<Component*> memory_children;
        for(Component* sm : root->GetChildren())
            if(sm->GetComponentType() == sys_sage::ComponentType::Subdivision && static_cast<Subdivision*>(sm)->GetSubdivisionType() == sys_sage::SubdivisionType::GpuSM)
                memory_children.push_back(sm);

        if((ret = mem->InsertBetweenParentAndChildren(root, memory_children, true)) != 0)
        {
            cerr << "parseMemory:InsertBetweenParentAndChildren failed with return code " << ret << endl;
        }
        
        if(latency != -1)
            for(Component* sm: memory_children)
                for(Component * c : sm->GetChildren())
                    if(c->GetComponentType() == sys_sage::ComponentType::Thread)
                        new DataPath(mem, c, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathType::Logical, 0, latency); 
    }
    else if(header_name == "SHARED_MEMORY") //very similar to parseCaches
    {   //shared memory is shared on an SM level
        if(shared_on != 1){
            std::cerr << "Mt4gParser::parseMemory, shared_on == 0 --> this should not happen...if yes, the implementation needs to be extended." << std::endl;
            return 1;
        }

        std::vector<Component*> parents;
        root->GetAllSubcomponentsByType(&parents, sys_sage::ComponentType::Subdivision);
        for(Component * parent : parents)
        {
            if(static_cast<Subdivision*>(parent)->GetSubdivisionType() == sys_sage::SubdivisionType::GpuSM)
            {
                if(!L2_shared_on_gpu)
                {
                    std::vector<Component*> caches = parent->GetAllChildrenByType(sys_sage::ComponentType::Cache);
                    for(Component * cache: caches){
                        if(static_cast<Cache*>(cache)->GetCacheName() == "L2"){
                            parent = cache;
                            break;
                        }
                    }
                }

                Memory * mem = new Memory(parent, 0, memory_name, (long long)size);

                //insert DP with latency
                if(latency != -1)
                {
                    std::vector<Component*> threads = parent->GetAllSubcomponentsByType(sys_sage::ComponentType::Thread);
                    for(Component* t: threads)
                        new DataPath(mem, t, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathType::Logical, 0, latency);
                }
            }
        }
    }
    else
    {
        std::cerr << "Mt4gParser::parseMemory, shared_on == 1 --> this should not happen...if yes, the implementation needs to be extended." << std::endl;
        return 1;
    }

    return 0;
}

int Mt4gParser::parseCaches(std::string header_name, std::string cache_type)
{
    std::vector<std::string> data = benchmarkData[header_name];
    data.erase(data.begin());

    //parse_args
    int shared_on = -1; //0=GPU, 1=SM
    int caches_per_sm = 1;
    double size = -1;
    int cache_line_size = -1;
    double latency = -1;
    int share_l1 = 0, share_texture = 0, share_ro = 0, share_constant = 0;
    int ret;
    for(size_t i = 0; i<data.size(); i++)
    {
        if(data[i]== "Size")
        {
            if(i>=data.size()-3){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            size = stod(data[i+1]);
            std::string unit = data[i+2];
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
            std::string unit = data[i+2];
            if(unit == "KiB")
                size *= 1024;
            else if(unit == "MiB")
                size *= 1024*1024;
            else if(unit == "GiB")
                size *= 1024*1024*1024;
            i+=2;
        }
        else if(data[i]== "Load_Latency")
        {
            if(i>=data.size()-2){
                cerr << "parseCaches: \"" << data[i] << "\" is supposed to be followed by 1 additional value." << endl;
                return 1;
            }
            if((data[i+2] == "cycles" && latency_in_cycles) || (data[i+2] == "nanoseconds" && !latency_in_cycles))
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
    { //shared on GPU level, place under main memory or L2(if no L2 available)
        Component * mem = root->GetChildByType(sys_sage::ComponentType::Memory);
        if(mem != NULL)
        {
            parent = mem;
            if(cache_type != "L2")
            {
                Component * l2 = mem->GetChildByType(sys_sage::ComponentType::Cache);
                if(static_cast<Cache*>(l2)->GetCacheName() == "L2")
                    parent = l2;
            }
        }
        Cache * cache = new Cache(parent, 0, cache_type);
        if(size != -1)
            cache->SetCacheSize(size);
        if(cache_line_size != -1)
            cache->SetCacheLineSize(cache_line_size);

        std::vector<Component*> sms;
        for(Component* sm : parent->GetChildren())
            if(sm->GetComponentType() == sys_sage::ComponentType::Subdivision && static_cast<Subdivision*>(sm)->GetSubdivisionType() == sys_sage::SubdivisionType::GpuSM)
                sms.push_back(sm);
        
        if((ret = cache->InsertBetweenParentAndChildren(parent, sms, true)) != 0)
        {
            cerr << "parseCaches:InsertBetweenParentAndChildren 1 failed with return code " << ret << endl;
        }
        
        //insert DP with latency
        if(latency != -1)
        {
            std::vector<Component*> threads = parent->GetAllSubcomponentsByType(sys_sage::ComponentType::Thread);
            for(Component* t: threads)
                new DataPath(cache, t, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathType::Logical, 0, latency);
        }
    }
    else if(shared_on == 1) //shared on SM
    {
        std::vector<Component*> sms = root->GetAllSubcomponentsByType(sys_sage::ComponentType::Subdivision);
        cout << endl << endl << endl << "=====      ===== ENTERING elseif for  - " << header_name << "sms (size=:" << sms.size() << endl << endl << endl;
        for(Component * sm : sms)
        {
            if(static_cast<Subdivision*>(sm)->GetSubdivisionType() == sys_sage::SubdivisionType::GpuSM)
            {
                Component* parent = sm;
                //if L2 is not shared on GPU, it will be the parent
                if(cache_type != "L2" && !L2_shared_on_gpu)
                {
                    std::vector<Component*> caches = parent->GetAllChildrenByType(sys_sage::ComponentType::Cache);
                    for(Component * cache: caches){
                        if(static_cast<Cache*>(cache)->GetCacheName() == "L2"){
                            parent = cache;
                            break;
                        }
                    }
                }
                //constant L1 is child of constant L1.5
                if(cache_type == "Constant_L1")
                {
                    std::vector<Component*> caches = parent->GetAllChildrenByType(sys_sage::ComponentType::Cache);
                    for(Component * cache: caches){
                        if(static_cast<Cache*>(cache)->GetCacheName() == "Constant_L1.5"){
                            parent = cache;
                            break;
                        }
                    }
                }

                std::vector<Component*> threads = sm->GetAllSubcomponentsByType(sys_sage::ComponentType::Thread);
                for(int i=0; i<caches_per_sm; i++)
                {
                    Cache * cache = new Cache(parent, i, cache_type);
                    if(size != -1)
                        cache->SetCacheSize(size);
                    if(cache_line_size != -1)
                        cache->SetCacheLineSize(cache_line_size);

                    int cores_per_cache = (*reinterpret_cast<int*>(root->attrib["Number_of_cores_per_SM"]))/caches_per_sm;

                    for(Component * thread : threads)
                    {
                        //if multiple caches per SM, move 1/n-th of threads (by their ID) under each cache
                        int core_id = thread->GetId();
                        if(core_id >= cores_per_cache*(i) && core_id < cores_per_cache*(i+1))
                        {
                            //for L1 and L2 caches, insert them between them and their cores as children
                            if((cache_type.find("L1") != std::string::npos && cache_type.find("Constant") == std::string::npos) || cache_type == "L2")
                            {
                                // cout << "   processing " << header_name << endl;
                                // cout << "cache_type=" << cache_type << ", Parent = " << parent->GetComponentTypeStr() << " " << parent->GetId() << endl; 
                                // for(Component * tc: parent->GetChildren())
                                // {
                                //     if(tc->GetComponentType() == ComponentType::Thread)
                                //         cout << ", " << tc->GetComponentTypeStr() << "(parent " << tc->GetParent()->GetComponentTypeStr() << ")";
                                // }
                                // cout << endl;
                                if((ret = cache->InsertBetweenParentAndChild(parent, thread, true)) != 0)
                                {
                                    //cerr << "parseCaches:InsertBetweenParentAndChild 2 failed with return code " << ret << endl;
                                }
                            }

                            if(latency != -1)
                                new DataPath(cache, thread, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathType::Logical, 0, latency);
                        }
                    }
                    cout << threads.size() << " - threads.size()" << ",   processing " << header_name << " == exited the (Component * thread : threads) " << endl;
                }
                //cout << "finished iterating over caches_per_sm:" << caches_per_sm << endl;
            }
        }
        cout << "=====      ===== FINISHED iterating over sms (size=:" << sms.size() << endl;
    }
    cout << endl << endl << endl << "FINISHING FUNCTION  - " << header_name << endl << endl << endl;
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

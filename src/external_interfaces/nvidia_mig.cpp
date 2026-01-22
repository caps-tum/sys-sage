#ifndef NVIDIA_MIG_CPP
#define NVIDIA_MIG_CPP

#include "defines.hpp"
#ifdef NVIDIA_MIG

#include <iostream>
#include <sstream>
#include <string>
#include <array>

#include <nvml.h>

#include "Component.hpp"
#include "Chip.hpp"
#include "Memory.hpp"
#include "Cache.hpp"
#include "Subdivision.hpp"


//SVTODO refactor to std::map<std::string, std::any> attrib;

//nvmlReturn_t nvmlDeviceGetMigDeviceHandleByIndex ( nvmlDevice_t device, unsigned int  index, nvmlDevice_t* migDevice ) --> look for all mig devices and add/update them
int sys_sage::Chip::UpdateMIGSettings(std::string uuid)
{
    int ret = 0;
    if(uuid.empty())
    {
        if(const char* env_p = std::getenv("CUDA_VISIBLE_DEVICES")){
            uuid = env_p;
        }
        if(uuid.empty()){
            std::cout << "Chip::UpdateMIGSettings: UUID is empty! Returning without updating the MIG settings." << std::endl;
            return 2;
        }
    }
	
    nvmlReturn_t nvml_ret = nvmlInit_v2();
    if(nvml_ret != NVML_SUCCESS){std::cerr << "Chip::UpdateMIGSettings: Couldn't initialize nvml. nvmlInit_v2 returns " << ret << ". Returning without updating the MIG settings." << std::endl; return 2;}

    nvmlDevice_t device;
    nvml_ret = nvmlDeviceGetHandleByUUID (uuid.c_str(), &device );
    if(nvml_ret != NVML_SUCCESS){std::cerr << "Chip::UpdateMIGSettings: nvmlDeviceGetHandleByUUID returns " << ret << ". Returning without updating the MIG settings." << std::endl; return 2;}
    
    nvmlDeviceAttributes_t attributes;
    nvml_ret = nvmlDeviceGetAttributes_v2 ( device, &attributes );
    if(nvml_ret != NVML_SUCCESS){std::cerr << "Chip::UpdateMIGSettings: nvmlDeviceGetAttributes_v2 returns " << ret << ". Returning without updating the MIG settings." << std::endl; return 2;}

    nvml_ret =  nvmlShutdown ( );
    if(nvml_ret != NVML_SUCCESS){std::cerr << "Chip::UpdateMIGSettings: nvmlShutdown returns " << ret << ". Will continue, though." << std::endl; ret = 1;}

    //cout << "...........multiprocessorCount " << attributes.multiprocessorCount << " gpuInstanceSliceCount=" << attributes.gpuInstanceSliceCount << "  computeInstanceSliceCount=" << attributes.computeInstanceSliceCount << "    memorySizeMB=" << attributes.memorySizeMB << endl;
    
    //main memory, expects the memory as a child of
    Memory* m = static_cast<Memory*>(GetChildByType(ComponentType::Memory));
    long long* mig_size;
    if(m != NULL){
        DataPath * d = NULL;
        //iterate over dp_outgoing to check if DP already exists
        for(Relation* r : GetRelationsByType(RelationType::DataPath))
        {
            DataPath * dp = static_cast<DataPath*>(r);
            if( dp->GetDataPathType() == DataPathType::MIG && *(static_cast<std::string*>(dp->attrib["mig_uuid"])) == uuid)
            {
                d = dp;
                break;
            }
        }

        d = new DataPath(this, m, DataPathOrientation::Bidirectional, DataPathType::MIG);
        std::string* mig_uuid = new std::string(uuid);
        mig_size = new long long(attributes.memorySizeMB*1000000);
        d->attrib.insert({"mig_uuid",reinterpret_cast<void*>(mig_uuid)});
        d->attrib.insert({"mig_size",reinterpret_cast<void*>(mig_size)});
    } else {
        std::cerr << "Chip::UpdateMIGSettings: Component Type Memory not found as a child of this Chip. Memory info will not be updated." << std::endl;
        ret = 1;
    }

    //L2 cache(s)
    unsigned int L2_fraction = 1; //which fraction of L2 is in MIG partition (the same fraction as the fraction of main memory)
    if(m->GetSize() > *mig_size){
        L2_fraction = (m->GetSize() + (*mig_size/2)) / *mig_size; //divide and round up or down
    }
    std::vector<Component*> caches;
    FindDescendantsByType(&caches, ComponentType::Cache);
    std::vector<Cache*> L2_caches;
    for(Component* c : caches){
        if(static_cast<Cache*>(c)->GetCacheName() == "L2"){
            L2_caches.push_back(static_cast<Cache*>(c));
        }            
    }
    int num_caches = L2_caches.size();
    if(num_caches > 0){
        int cache_id = 0;
        for(Cache* c : L2_caches){
            DataPath * d = new DataPath(this, c, DataPathOrientation::Bidirectional, DataPathType::MIG);
            std::string* mig_uuid = new std::string(uuid);
            mig_size = new long long();
            *mig_size = c->GetCacheSize() * ( static_cast<float>(num_caches)/static_cast<float>(L2_fraction)-static_cast<float>(cache_id)/static_cast<float>(num_caches));
            if(*mig_size <0)
                *mig_size=0;
            d->attrib.insert({"mig_uuid",reinterpret_cast<void*>(mig_uuid)});
            d->attrib.insert({"mig_size",reinterpret_cast<void*>(mig_size)});
            cache_id++;
        }
    } else {
        std::cerr << "Chip::UpdateMIGSettings: L2 Cache component not found as a child of this Chip. L2 size info will not be updated." << std::endl;
        ret = 1;
    }

    //sm  attributes.multiprocessorCount
    std::vector<Component*> subdivisions;
    FindDescendantsByType(&subdivisions, ComponentType::Subdivision);
    std::vector<Subdivision*> sms;
    for(Component* sm : subdivisions){
        if(static_cast<Subdivision*>(sm)->GetSubdivisionType() == SubdivisionType::GpuSM)
            sms.push_back(static_cast<Subdivision*>(sm));
    }
    for(Subdivision* sm: sms){
        if(sm->GetId() < static_cast<int>(attributes.multiprocessorCount)){
            DataPath * d = new DataPath(this, sm, DataPathOrientation::Bidirectional, DataPathType::MIG);
            std::string* mig_uuid = new std::string(uuid);
            d->attrib.insert({"mig_uuid",reinterpret_cast<void*>(mig_uuid)});
        }
    }

    return ret;
}

int sys_sage::Chip::GetMIGNumSMs(std::string uuid)
{
    if(uuid.empty()){
        if(const char* env_p = std::getenv("CUDA_VISIBLE_DEVICES")){
            uuid = env_p;
        }
    }
    int num_sm = 0;
    if(uuid.empty()) //when no uuid provided and no uuid found in env CUDA_VISIBLE_DEVICES, return full GPU num SMs.
    {
        std::cerr << "Chip::GetMIGNumSMs: no UUID provided or found in env CUDA_VISIBLE_DEVICES. Returning information for full machine." << std::endl;
        
        std::vector<Component*> subdivisions;
        FindDescendantsByType(&subdivisions, ComponentType::Subdivision);
        std::vector<Subdivision*> sms;
        for(Component* sm : subdivisions){
            if(static_cast<Subdivision*>(sm)->GetSubdivisionType() == SubdivisionType::GpuSM){
                num_sm++;
            }
        }
    } 
    else
    {
        for(Relation* r : GetRelationsByType(RelationType::DataPath))
        {
            DataPath * dp = static_cast<DataPath*>(r);
            if(dp->GetDataPathType() == DataPathType::MIG && *reinterpret_cast<std::string*>(dp->attrib["mig_uuid"]) == uuid){
                Component* target = dp->GetTarget();
                if(target->GetComponentType() == ComponentType::Subdivision && static_cast<Subdivision*>(target)->GetSubdivisionType() == SubdivisionType::GpuSM ){
                    num_sm++;
                }
            }
        }
    }
    return num_sm;
}

int sys_sage::Chip::GetMIGNumCores(std::string uuid)
{
    std::vector<Subdivision*> sms;
    std::vector<Component*> cores;
    if(uuid.empty()){
        if(const char* env_p = std::getenv("CUDA_VISIBLE_DEVICES")){
            uuid = env_p;
        }
    }

    if(uuid.empty()) //when no uuid provided and no uuid found in env CUDA_VISIBLE_DEVICES, return full GPU num SMs.
    {
        std::cerr << "Chip::GetMIGNumCores: no UUID provided or found in env CUDA_VISIBLE_DEVICES. Returning information for full machine." << std::endl;

        std::vector<Component*> subdivisions;
        FindDescendantsByType(&subdivisions, ComponentType::Subdivision);
        for(Component* sm : subdivisions){
            if(static_cast<Subdivision*>(sm)->GetSubdivisionType() == SubdivisionType::GpuSM)
                sms.push_back(static_cast<Subdivision*>(sm));
        }
    }
    else
    {
        for(Relation* r : GetRelationsByType(RelationType::DataPath))
        {
            DataPath * dp = static_cast<DataPath*>(r);
            if(dp->GetDataPathType() == DataPathType::MIG && *reinterpret_cast<std::string*>(dp->attrib["mig_uuid"]) == uuid){
                Component* target = dp->GetTarget();
                if(target->GetComponentType() == ComponentType::Subdivision && static_cast<Subdivision*>(target)->GetSubdivisionType() == SubdivisionType::GpuSM ){
                    sms.push_back(static_cast<Subdivision*>(target));
                }
            }
        }
    }

    for(Subdivision* sm: sms)
    {
        sm->FindDescendantsByType(&cores, ComponentType::Thread);
    }
    
    return cores.size();
}

long long sys_sage::Memory::GetMIGSize(std::string uuid) const
{
    if(uuid.empty()){
        if(const char* env_p = std::getenv("CUDA_VISIBLE_DEVICES")){
            uuid = env_p;
        }
    }

    if(uuid.empty()) //when no uuid provided and no uuid found in env CUDA_VISIBLE_DEVICES, return full GPU num SMs.
    {
        std::cerr << "Memory::GetMIGSize: no UUID provided or found in env CUDA_VISIBLE_DEVICES. Returning information for full machine." << std::endl;
        return size;
    } 

    for(Relation* r : GetRelationsByType(RelationType::DataPath))
    {
        DataPath * dp = static_cast<DataPath*>(r);
        if(dp->GetDataPathType() == DataPathType::MIG && *reinterpret_cast<std::string*>(dp->attrib["mig_uuid"]) == uuid){
            if (dp->attrib.count("mig_size")){
                long long r = *reinterpret_cast<long long*>(dp->attrib["mig_size"]);
                return r;
            }
        }
    }
    std::cerr << "Memory::GetMIGSize: no information found about specified UUID " << uuid << " - returning full memory size." << std::endl;
    return size; 
}

long long sys_sage::Cache::GetMIGSize(std::string uuid) const
{
    if(uuid.empty()){
        if(const char* env_p = std::getenv("CUDA_VISIBLE_DEVICES")){
            uuid = env_p;
        }
    }

    if(uuid.empty()) //when no uuid provided and no uuid found in env CUDA_VISIBLE_DEVICES, return full GPU num SMs.
    {
        std::cerr << "Cache::GetMIGSize: no UUID provided or found in env CUDA_VISIBLE_DEVICES. Returning information for full machine." << std::endl;
        return cache_size;
    }

    if(GetCacheLevel() == 2){
        for(Relation* r : GetRelationsByType(RelationType::DataPath))
        {
            DataPath * dp = static_cast<DataPath*>(r);
            if(dp->GetDataPathType() == DataPathType::MIG && *reinterpret_cast<std::string*>(dp->attrib["mig_uuid"]) == uuid){
                if (dp->attrib.count("mig_size")){
                    long long r = *reinterpret_cast<long long*>(dp->attrib["mig_size"]);
                    return r;
                }
            }
        }
    }
    std::cerr << "Cache::GetMIGSize: no information found about specified UUID " << uuid << " - returning full cache size." << std::endl;
    return cache_size;
}

#endif
#endif

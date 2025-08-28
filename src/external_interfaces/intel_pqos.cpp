#ifndef INTEL_PQOS_CPP
#define INTEL_PQOS_CPP

#include "defines.hpp"
#ifdef INTEL_PQOS
//code written based on https://github.com/intel/intel-cmt-cat/blob/master/examples/c/CAT_MBA/allocation_app_l3cat.c

#include <pqos.h>
#include <cstring> //memset
#include <limits> //numeric_limits

#include "Component.hpp"

using namespace std;

//l3cat_ids, l3cat_id_count
uint64_t getCOSL3Bitmask(unsigned int socketId, unsigned int catCosId, unsigned * socketIdArray, unsigned socketIdArraySz)
{
    struct pqos_l3ca tab[PQOS_MAX_L3CA_COS]; //TODO fill this once per socket and reuse

    //find index of my socket in the socketIdArray
    int socketIndex = -1;
    for(unsigned i = 0; i< socketIdArraySz; i++){
        if (socketIdArray[i] == socketId){
            socketIndex = i;
            break;
        }
    }
    if(socketIndex == -1)
        return std::numeric_limits<uint64_t>::max();

    unsigned num = 0;
    int ret = pqos_l3ca_get(socketIndex, PQOS_MAX_L3CA_COS, &num, tab);
    if (ret == PQOS_RETVAL_OK)
    {
        for (unsigned n = 0; n < num; n++) {
            if(tab[n].class_id == catCosId){
                //cout << "-----returning socket " << socketId << " COS " << catCosId << " - found mask 0x" << hex << tab[n].u.ways_mask << dec << endl;
                return tab[n].u.ways_mask;
            }
        }
    }
    return std::numeric_limits<uint64_t>::max();
}

uint64_t getCoreCOS(unsigned int socketId, unsigned int coreId, unsigned * socketIdArray, unsigned socketIdArraySz, const struct pqos_cpuinfo *p_cpu)
{
    //find index of my socket in the socketIdArray
    int socketIndex = -1;
    for(unsigned i = 0; i< socketIdArraySz; i++){
        if (socketIdArray[i] == socketId){
            socketIndex = i;
            break;
        }
    }
    if(socketIndex == -1)
        return std::numeric_limits<uint64_t>::max();

    unsigned coreIdArraySz = 0;
    unsigned *coreIdArray = pqos_cpu_get_cores(p_cpu, socketIndex, &coreIdArraySz);
    if (coreIdArray == NULL || coreIdArray == 0) {
        printf("Error retrieving core information!\n");
        free(coreIdArray);
        free(socketIdArray);
        return std::numeric_limits<uint64_t>::max();
    }

    //find index of my core in coreIdArray
    int coreIndex = -1;
    for(unsigned i = 0; i< coreIdArraySz; i++){
        if (coreIdArray[i] == coreId){
            coreIndex = i;
            break;
        }
    }
    if(coreIndex == -1)
        return std::numeric_limits<uint64_t>::max();

    unsigned class_id = 0;
    int ret = pqos_alloc_assoc_get(coreIndex, &class_id);
    if (ret == PQOS_RETVAL_OK){
        //cout << "--returning socket " << socketId << " core " << coreId << "COS " << class_id << endl;
        return class_id;
    }
    return std::numeric_limits<uint64_t>::max();
}

int Node::UpdateL3CATCoreCOS(){

    struct pqos_config cfg;
    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;
    unsigned l3cat_id_count, *p_l3cat_ids = NULL;
    int ret;

    memset(&cfg, 0, sizeof(cfg));

    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    ret = pqos_init(&cfg);
    if (ret != PQOS_RETVAL_OK) {
            std::cerr << "Error initializing PQoS library!" << std::endl;
            return 0;
    }
    /* Get CMT capability and CPU info pointer */
    ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
            std::cerr << "Error retrieving PQoS capabilities!" << std::endl;
            return 0;
    }
    /* Get CPU l3cat id information to set COS */
    p_l3cat_ids = pqos_cpu_get_l3cat_ids(p_cpu, &l3cat_id_count);
    if (p_l3cat_ids == NULL) {
            std::cerr << "Error retrieving CPU socket information!" << std::endl;
            return 0;
    }

    vector<Chip*> sockets;
    GetSubcomponentsByType(reinterpret_cast<vector<Component*>*>(&sockets), sys_sage::ComponentType::Chip);
    for(auto it = std::begin(sockets); it != std::end(sockets); ++it)
    {
        Chip* socket = *it;
        //std::cout << "socket " << socket->GetComponentTypeStr() << " id " << socket->GetId() << std::endl;
        vector<Thread*> threads;
        socket->GetSubcomponentsByType(reinterpret_cast<vector<Component*>*>(&threads), sys_sage::ComponentType::Thread);
        for(auto it_threads = std::begin(threads); it_threads != std::end(threads); ++it_threads)
        {
            Thread* thread = *it_threads;
            //std::cout << "  thread " << thread->GetComponentTypeStr() << " id " << thread->GetId() << std::endl;
            uint64_t* cos = new uint64_t();
            uint64_t* mask = new uint64_t();
            *cos = getCoreCOS(socket->GetId(), thread->GetId(), p_l3cat_ids, l3cat_id_count, p_cpu);
            if(*cos == std::numeric_limits<uint64_t>::max()){
                cerr << "getCoreCOS failed" << endl;
                continue;
            }
            *mask = getCOSL3Bitmask(socket->GetId(), *cos, p_l3cat_ids, l3cat_id_count);
            if(*mask == std::numeric_limits<uint64_t>::max()){
                cerr << "getCOSL3Bitmask failed" << endl;
                continue;
            }

            //find L3 cache above the threads
            Component* c = thread;
            while(c->GetParent() != NULL){
                //go up until L3 found
                c = c->GetParent();
                if(c->GetComponentType() == sys_sage::ComponentType::Cache && static_cast<Cache*>(c)->GetCacheLevel() == 3)
                    break;
            };
            if(c==NULL || c->GetComponentType() != sys_sage::ComponentType::Cache){
                cerr << "L3 cache not found" << endl; continue;
            }

            //TODO check if exists -> overwrite

            //add DataPath to thread and L3
            DataPath* d = NewDataPath(thread, c, SYS_SAGE_DATAPATH_BIDIRECTIONAL, SYS_SAGE_DATAPATH_TYPE_L3CAT);
            d->attrib.insert({"CATcos", reinterpret_cast<void*>(cos)});
            d->attrib.insert({"CATL3mask", reinterpret_cast<void*>(mask)});
        }
    }
    return 1;
}

long long Thread::GetCATAwareL3Size()
{
    //look for dp_outgoing where attrib contains "CATL3mask"
    for(auto it = std::begin(dp_outgoing); it != std::end(dp_outgoing); ++it)
    {
        DataPath* dp = *it;
        auto search = dp->attrib.find("CATL3mask");
        if (search == dp->attrib.end()) {
            continue;
        }
        uint64_t* mask = reinterpret_cast<uint64_t*>(search->second);

        Cache* c = dynamic_cast<Cache*>(dp->GetTarget());
        if (!c) {
            continue;
        }
        int available_cache_associativity_ways = 0;
        for(int bit = 0; bit<c->GetCacheAssociativityWays(); bit++){
            if((*mask & (1ULL<<bit)) == (1ULL<<bit)){
                available_cache_associativity_ways++;
            }
        }
        //cout << "GetCATAwareL3Size: size " << c->GetCacheSize() << " tot_ways " << c->GetCacheAssociativityWays() << ", available ways " << available_cache_associativity_ways << endl;
        return c->GetCacheSize() / c->GetCacheAssociativityWays() * available_cache_associativity_ways ;
    }

    Component* c = this;
    while(c->GetParent() != NULL){
        //go up until L3 found
        c = c->GetParent();
        if(c->GetComponentType() == sys_sage::ComponentType::Cache && static_cast<Cache*>(c)->GetCacheLevel() == 3)
            return static_cast<Cache*>(c)->GetCacheSize();
    };
    return -1;
}

#endif //INTEL_PQOS
#endif //INTEL_PQOS_CPP

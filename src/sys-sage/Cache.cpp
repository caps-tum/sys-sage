#include "Cache.hpp"


sys_sage::Cache::Cache(int _id, int  _cache_level, long long _cache_size, int _associativity, int _cache_line_size): Component(_id, "Cache", sys_sage::ComponentType::Cache), cache_type(std::to_string(_cache_level)), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
sys_sage::Cache::Cache(Component * parent, int _id, std::string _cache_type, long long _cache_size, int _associativity, int _cache_line_size): Component(parent, _id, "Cache", sys_sage::ComponentType::Cache), cache_type(_cache_type), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
sys_sage::Cache::Cache(Component * parent, int _id, int _cache_level, long long _cache_size, int _associativity, int _cache_line_size): Cache(parent, _id, std::to_string(_cache_level), _cache_size, _associativity, -1){}



const std::string& sys_sage::Cache::GetCacheName() const{return cache_type;}
void sys_sage::Cache::SetCacheName(std::string _name) { cache_type = _name;}

int sys_sage::Cache::GetCacheLevel() const{

    std::string extractedDigits = "";
    for (char c : cache_type) {
        // Break, as soon as a digit is found
        if (std::isdigit(c)) {
            extractedDigits += c;
            break;
        }
    }

    if (!extractedDigits.empty()) 
        return stoi(extractedDigits);
    else 
        return 0;
    
}

void sys_sage::Cache::SetCacheLevel(int _cache_level) { cache_type = std::to_string(_cache_level); }
long long sys_sage::Cache::GetCacheSize() const {return cache_size;}
void sys_sage::Cache::SetCacheSize(long long _cache_size){cache_size = _cache_size;}
int sys_sage::Cache::GetCacheLineSize() const{return cache_line_size;}
void sys_sage::Cache::SetCacheLineSize(int _cache_line_size){cache_line_size = _cache_line_size;}
int sys_sage::Cache::GetCacheAssociativityWays() const {return cache_associativity_ways;}
void sys_sage::Cache::SetCacheAssociativityWays(int _associativity) { cache_associativity_ways = _associativity;}


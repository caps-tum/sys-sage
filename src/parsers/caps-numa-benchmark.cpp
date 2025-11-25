
#include "caps-numa-benchmark.hpp"

#include <iostream>
#include <fstream>
#include <vector>


int sys_sage::parseCapsNumaBenchmark(Component* rootComponent, std::string benchmarkPath, std::string delim)
{
    CSVReader reader(benchmarkPath, delim);
    std::vector<std::vector<std::string> > benchmarkData;
    if(reader.getData(&benchmarkData) != 0) {//Error
        std::cerr << "error: could not parse CapsNumaBenchmark file " << benchmarkPath.c_str() << std::endl;
        return 1;
    }

    //get indexes of relevant columns
    int cpu_is_source=-1;//-1 initial, 0 numa is source, 1 cpu is source
    std::vector<std::string> header = benchmarkData[0];
    int src_cpu_idx=-1;
    int src_numa_idx=-1;
    int target_numa_idx=-1;
    int ldlat_idx=-1;
    int bw_idx=-1;
    for(unsigned int i=0; i<header.size(); i++)
    {
        if(header[i] == "src_cpu")
            src_cpu_idx=i;
        else if(header[i] == "src_numa")
            src_numa_idx=i;
        else if(header[i] == "target_numa")
            target_numa_idx=i;
        else if(header[i] == "ldlat(ns)")
            ldlat_idx=i;
        else if(header[i] == "bw(MB/s)")
            bw_idx=i;
    }
    if(src_cpu_idx > -1)
        cpu_is_source += 2;
    if(src_numa_idx > -1)
        cpu_is_source += 1;

    if(cpu_is_source==-1 || cpu_is_source>1 || target_numa_idx==-1 || ldlat_idx==-1 || bw_idx==-1){
        std::cerr << "indexes: " << src_cpu_idx << src_numa_idx << target_numa_idx << ldlat_idx << bw_idx << std::endl;
        return 1;
    }

    //cout << "caps-numa-benchmark parser: num entries: " << benchmarkData.size()-1 << endl;
    //parse each line as one DataPath, skip header
    for(unsigned int i=1; i<benchmarkData.size(); i++)
    {
        int src_cpu_id, src_numa_id, target_numa_id;
        unsigned long long bw, ldlat;
        Component *src, *target;

        if(cpu_is_source){
            src_cpu_id = stoi(benchmarkData[i][src_cpu_idx]);
            src = rootComponent->GetDescendantById(src_cpu_id, sys_sage::ComponentType::Thread);
        }else{
            src_numa_id = stoi(benchmarkData[i][src_numa_idx]);
            src = rootComponent->GetDescendantById(src_numa_id, sys_sage::ComponentType::Numa);
        }
        target_numa_id = stoi(benchmarkData[i][target_numa_idx]);
        target = rootComponent->GetDescendantById(target_numa_id, sys_sage::ComponentType::Numa);
        if(src == NULL || target == NULL)
            std::cerr << "error: could not find components; skipping " << std::endl;
        else{
            bw = stoul(benchmarkData[i][bw_idx]);
            ldlat = stoul(benchmarkData[i][ldlat_idx]);

            new DataPath(src, target, sys_sage::DataPathOrientation::Oriented, sys_sage::DataPathType::Datatransfer, static_cast<double>(bw), static_cast<double>(ldlat));

        }
    }
    return 0;
}

int sys_sage::CSVReader::getData(std::vector<std::vector<std::string> >* dataList)
{
    std::ifstream file(benchmarkPath);
    if (!file.good())
        return 1;
    std::string line = "";
    while (getline(file, line))
    {
        std::vector<std::string> vec;
        size_t pos = 0;
        while ((pos = line.find(delimiter)) != std::string::npos) {
            vec.push_back(line.substr(0, pos));
            //cout << line.substr(0, pos) << "_";
            line.erase(0, pos + delimiter.length());
        }
        //insert the rest of the line after the last delim.
        vec.push_back(line.substr(0, pos));
        //cout << line.substr(0, pos) << endl;
        dataList->push_back(vec);
    }
    // Close the File
    file.close();
    return 0;
}

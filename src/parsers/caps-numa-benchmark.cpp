
#include "caps-numa-benchmark.hpp"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int parseCapsNumaBenchmark(Component* rootComponent, string benchmarkPath, string delim = ";")
{
    CSVReader reader(benchmarkPath, delim);
    vector<vector<string> > benchmarkData;
    if(reader.getData(&benchmarkData) != 0) {//Error
        cerr << "error: could not parse CapsNumaBenchmark file " << benchmarkPath.c_str() << endl;
        return 1;
    }

    //get indexes of relevant columns
    int cpu_is_source=-1;//-1 initial, 0 numa is source, 1 cpu is source
    vector<string> header = benchmarkData[0];
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
        cerr << "indexes: " << src_cpu_idx << src_numa_idx << target_numa_idx << ldlat_idx << bw_idx << endl;
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
            src = rootComponent->FindSubcomponentById(src_cpu_id, SYS_SAGE_COMPONENT_THREAD);
        }else{
            src_numa_id = stoi(benchmarkData[i][src_numa_idx]);
            src = rootComponent->FindSubcomponentById(src_numa_id, SYS_SAGE_COMPONENT_NUMA);
        }
        target_numa_id = stoi(benchmarkData[i][target_numa_idx]);
        target = rootComponent->FindSubcomponentById(target_numa_id, SYS_SAGE_COMPONENT_NUMA);
        if(src == NULL || target == NULL)
            cerr << "error: could not find components; skipping " << endl;
        else{
            bw = stoul(benchmarkData[i][bw_idx]);
            ldlat = stoul(benchmarkData[i][ldlat_idx]);

            new DataPath(src, target, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_DATATRANSFER, (double)bw, (double)ldlat);

        }
    }
    return 0;
}

int CSVReader::getData(vector<vector<string> >* dataList)
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

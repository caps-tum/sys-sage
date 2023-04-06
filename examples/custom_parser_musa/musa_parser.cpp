#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "musa_parser.hpp"

int parseMusa(Chip* _socket, std::string datapath) {

	MusaParser parse(_socket, datapath);
	int ret = parse.ParseData();

	if (ret) {
		std::cout << " File " << datapath << " couldn't be parsed." << std::endl;
	}
    return ret;
}

MusaParser::MusaParser(Chip* _socket, std::string _datapath) {
	datapath = _datapath;
    socket = _socket;
}

std::ostream& operator<<(std::ostream& output, std::map<std::string, double>& map) {
	std::map<std::string, double>::iterator that = map.begin();


	// Iterate through the map and print the elements
	while (that != map.end())
	{
		output << "Key: " << that->first << ", Value: " << that->second << std::endl;
		++that;
	}
	return output;
}

int MusaParser::ReadData(std::vector<std::string> search) {
	std::ifstream myfile(datapath);
	if (!myfile.good()) {
		std::cout << " Couldn't open data source path " << datapath << std::endl;
		return 1;
	}
	std::string line, subline, word;
	int offset;
	std::string value;
	for (const auto & element : search) {
		myfile.clear();
		myfile.seekg(0, myfile.beg);
		while (!myfile.eof()) {
			std::getline(myfile, line);
			if ((offset = line.find(element, 0) != std::string::npos)) {
				std::map<std::string, std::string> input;
				do {
					std::getline(myfile, subline);
					subline.erase(remove(subline.begin(), subline.end(), '='), subline.end());
					std::istringstream ss(subline);
					ss >> word >> value;
					input.insert({ word,value });
				} while (!subline.empty());

				mapping.insert({ element, input });
			}
		}
	}

	return 0;
}

int MusaParser::ParseData() {
	std::vector<std::string> search = { "DL1Cache", "Global", "Memory", "L2Cache", "L3Cache", "RAMULATOR"};
	int ret = ReadData(search);
	if (ret != 0) {
		std::cout << "Was not able to parse information from file" << std::endl;
		return 1;
	}
    int memory_found=1;
	for (auto element : search) {
        if (mapping.find(element) == mapping.end()) {
            if(element == "Memory") //memory is not mandatory -- it only contains bw,lat information..so proceed if not found
                memory_found = 0;
            else{
                std::cout << " Couldn't find " << element << " information in file" << std::endl;
                return 1;
            }
		}
	}

	Memory* mem = ParseMemory();
    double main_mem_bw=0, main_mem_lat=0;
    if(memory_found){
        main_mem_bw = stod(mapping["Memory"]["bandwidth"]);
        main_mem_lat = stod(mapping["Memory"]["latency"]);
    }
    double l1_lat = stod(mapping["DL1Cache"]["latency"]);
    double l2_lat = stod(mapping["L2Cache"]["latency"]);
    double l3_lat = stod(mapping["L3Cache"]["latency"]);

	Cache* l3cache = ParseCache("L3Cache", mem);
	int ncpus = stoi(mapping["Global"]["ncpus"]);
	int nthreads= stoi(mapping["Global"]["threads_per_cpu"]);
    int coreId = 0, threadId = 0;
	for (int i = 0; i < ncpus; i++) {
		Cache* l2cache = ParseCache("L2Cache", l3cache);
		Cache* l1cache = ParseCache("DL1Cache", l2cache);
		Core* core = new Core(l1cache, coreId);
        coreId++;
		for (int i = 0; i < nthreads; i++) {
			Thread* thread = new Thread(core, threadId);
            threadId ++;

            if(memory_found){
                new DataPath(mem, thread, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_DATATRANSFER, main_mem_bw, main_mem_lat);
            }
            new DataPath(l1cache, thread, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_DATATRANSFER, 0, l1_lat);
            new DataPath(l2cache, thread, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_DATATRANSFER, 0, l2_lat);
            new DataPath(l3cache, thread, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_DATATRANSFER, 0, l3_lat);
		}
	}

	return 0;
}

Memory* MusaParser::ParseMemory() {
    // parsing memory size from the
	std::string input = mapping["RAMULATOR"]["org"];
	int pos1 = input.find("_");
	int pos2 = input.find("_", pos1+1);
	std::string output;
    if(pos2 == std::string::npos) //no second underscore
        output = input.substr(pos1 + 1);
    else
        output = input.substr(pos1 + 1, pos2 - pos1 - 1);
	long long size = stol(output);
    long long channels = stol(mapping["RAMULATOR"]["channels"]);
    if(channels > 0)
        size *= channels;

    std::transform(output.begin(), output.end(), output.begin(),
        [](unsigned char c){ return std::tolower(c); }); //convert to lower case for comparison
    if (output.find("kib") != std::string::npos)
        size *= (long long)1024;
    else if (output.find("mib") != std::string::npos)
        size *= (long long)1024*(long long)1024;
    else if (output.find("gib") != std::string::npos)
        size *= (long long)1024*(long long)1024*(long long)1024;
    else if (output.find("tib") != std::string::npos)
        size *= (long long)1024*(long long)1024*(long long)1024*(long long)1024;
    else if (output.find("kb") != std::string::npos)
        size *= (long long)1000;
    else if (output.find("mb") != std::string::npos)
        size *= (long long)1000*(long long)1000;
    else if (output.find("gb") != std::string::npos)
        size *= (long long)1000*(long long)1000*(long long)1000;
    else if (output.find("tb") != std::string::npos)
        size *= (long long)1000*(long long)1000*(long long)1000*(long long)1000;

    Memory* mem = new Memory(socket, input, size);
	return mem;
}

Cache* MusaParser::ParseCache(std::string level, Component* parent) {
	std::map<std::string, std::string> data = mapping[level];

    unsigned long long cache_size = stoul(data["size"]);
    int cache_line_size = stoi(data["line-size"]);
    int associativity = stoi(data["assoc"]);
    int cache_level = stoi(data["level"]);

    Cache* cache = new Cache(parent, 0/*id*/, cache_level, cache_size, associativity, cache_line_size);
	return cache;
}

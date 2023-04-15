#ifndef MUSA_PARSER
#define MUSA_PARSER

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>


#include "sys-sage.hpp"


int parseMusa(Chip* _socket, std::string datapath);

class MusaParser {
public:
	int ParseData();
	MusaParser(Chip* _socket, std::string _datapath);

private:
	int ReadData(std::vector<std::string>);
	Memory* ParseMemory();
	Cache* ParseCache(std::string, Component* parent);

	std::map<std::string, std::map<std::string, std::string>> mapping;
	std::string datapath;
	Chip* socket;
};

#endif

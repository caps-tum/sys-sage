#ifndef MUSA_PARSER
#define MUSA_PARSER

#include <sys-sage.hpp>
#include <string>
#include <vector>
#include <map>

int parseMusa(sys_sage::Chip* _socket, std::string datapath);

class MusaParser {
public:
	int ParseData();
	MusaParser(sys_sage::Chip* _socket, std::string _datapath);

private:
	int ReadData(std::vector<std::string>);
	sys_sage::Memory* ParseMemory();
	sys_sage::Cache* ParseCache(std::string, sys_sage::Component* parent);

	std::map<std::string, std::map<std::string, std::string>> mapping;
	std::string datapath;
	sys_sage::Chip* socket;
};

#endif

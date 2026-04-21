#ifndef SYS_SAGE_PARSERS_CAPS_NUMA_BENCHMARK_HPP
#define SYS_SAGE_PARSERS_CAPS_NUMA_BENCHMARK_HPP

#include <string>
#include <vector>

namespace sys_sage {
    class Component;
};

namespace sys_sage {
    int parseCapsNumaBenchmark(Component* rootComponent, std::string benchmarkPath, std::string delim = ";");

    class CSVReader
    {
        std::string benchmarkPath;
        std::string delimiter;
    public:
        CSVReader(std::string benchmarkPath, std::string delm = ";") : benchmarkPath(benchmarkPath), delimiter(delm) { }
        // Function to fetch data from a CSV File
        int getData(std::vector<std::vector<std::string> >*);
    };

} //namespace sys_sage

#endif

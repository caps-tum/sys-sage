#ifndef CAPS_NUMA_BENCHMARK
#define CAPS_NUMA_BENCHMARK

#include <sys-sage/Component.hpp>
#include <sys-sage/DataPath.hpp>


namespace sys_sage {
    /**
     * @brief Parser for the caps-numa-benchmark data source.
     *
     * @param rootComponent The root of the topology.
     * @param benchmarkPath The path to the caps-numa-benchmark CSV output file.
     * @param delim The delimiter used in the CSV file.
     *
     * @return 0 on success, 1 on failure.
     *
     * @pythonBinding
     */
    int parseCapsNumaBenchmark(Component* rootComponent, const std::string &benchmarkPath, const std::string &delim = ";");

    /**
     * @private
     */
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

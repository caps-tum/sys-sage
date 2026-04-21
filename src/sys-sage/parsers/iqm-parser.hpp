/**
* @file iqm-parser.hpp
* @brief sys-sage's interface to IQM Q-Systems.
*/

#ifndef SYS_SAGE_PARSERS_IQM_PARSER_HPP
#define SYS_SAGE_PARSERS_IQM_PARSER_HPP

#include <nlohmann/json.hpp>
#include <string>

namespace sys_sage {
    class Component;
    class QuantumBackend;
}

using json = nlohmann::json;

namespace sys_sage {
    //user calls only these functions
    int parseIQM(Component* parent, std::string dataSourcePath, int qcId, int tsForHistory = -1);
    int parseIQM(QuantumBackend* parent, std::string dataSourcePath, int qcId, int tsForHistory = -1, bool createTopo = true);

    class IQMParser
    {
    public: 

        IQMParser(QuantumBackend* _qc,std::string filepath);
        int CreateQcTopo();
        int ParseDynamicData(int tsForHistory);
    private:
        double t1_max, t2_max, q1_fidelity_max, readout_fidelity_max;

        json jsonData;
        QuantumBackend * backend;
    };
} //namespace sys_sage

#endif
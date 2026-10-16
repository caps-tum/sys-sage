/**
* @file iqm-parser.hpp
* @brief sys-sage's interface to IQM Q-Systems.
*/

#ifndef IQM_PARSER_HPP
#define IQM_PARSER_HPP

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <sys-sage/Component.hpp>
#include <sys-sage/QuantumBackend.hpp>
#include <sys-sage/Qubit.hpp>
#include <sys-sage/CouplingMap.hpp>

namespace sys_sage {
    //user calls only these functions
    /**
     * @brief Parser for the IQM data source.
     *
     * @param parent
     * @param dataSourcePath
     * @param qcId
     * @param tsForHistory
     *
     * @pythonBinding
     */
    int parseIQM(Component* parent, const std::string &dataSourcePath, int qcId, int tsForHistory = -1);

    /**
     * @brief Parser for the IQM data source.
     *
     * @param parent
     * @param dataSourcePath
     * @param qcId
     * @param tsForHistory
     * @param createTopo
     *
     * @pythonBinding
     */
    int parseIQM(QuantumBackend* parent, const std::string &dataSourcePath, int qcId, int tsForHistory = -1, bool createTopo = true);

    /**
     * @private
     */
    class IQMParser
    {
    public: 

        IQMParser(QuantumBackend* _qc, const std::string &filepath);
        int CreateQcTopo();
        int ParseDynamicData(int tsForHistory);
    private:
        double t1_max, t2_max, q1_fidelity_max, readout_fidelity_max;

        nlohmann::json jsonData;
        QuantumBackend * backend;
    };
} //namespace sys_sage

#endif // IQM_PARSER_HPP
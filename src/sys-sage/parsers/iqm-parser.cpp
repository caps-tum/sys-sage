/**
 * @file iqm-parser.cpp
 * @brief sys-sage's interface to IQM
 */
#include "iqm-parser.hpp"
#include <algorithm>
#include <sstream>


int sys_sage::parseIQM(Component* parent, std::string dataSourcePath, int qcId, int tsForHistory)
{
    if(parent == NULL){
        std::cerr << "parseIQM: parent is null" << std::endl;
        return 1;
    }
    QuantumBackend* qc = new QuantumBackend(parent,qcId);
    return parseIQM(qc, dataSourcePath, qcId, tsForHistory, true);   
}

int sys_sage::parseIQM(QuantumBackend* qc, std::string dataSourcePath, int qcId, int tsForHistory, bool createTopo)
{
    IQMParser iqm(qc,dataSourcePath);
    int ret;
    //only creates qubits & coupling mappings; no dynamic info
    if(createTopo)
    {
        ret = iqm.CreateQcTopo();
        if(ret != 0)
            return ret;
    }
    
    //assumes that the qubits and coupling mappings are already in place
    ret = iqm.ParseDynamicData(tsForHistory);
    return ret;
}

sys_sage::IQMParser::IQMParser(QuantumBackend* _qc,std::string filepath)
{ 
    backend = _qc;

    std::ifstream file(filepath);
    if (!file.is_open()) 
    {
        throw std::runtime_error("Failed to open file!");
    }

    jsonData = json::parse(file);
}

int sys_sage::IQMParser::CreateQcTopo()
{
    backend->SetName(jsonData["backend_name"]);
    int numQubits = jsonData["T1"].size();

    backend->SetNumQubits(numQubits);
    
    auto parse_pair = [](const std::string& pair_str) -> std::pair<int, int>
    {
        std::stringstream ss(pair_str);
        int x, y;
        char comma;
        ss >> x >> comma >> y;
        return {x, y};
    };

    for(int i=0; i<numQubits; i++)
    {
        new Qubit(backend, i);
    }
    
    std::map<std::string, std::string> two_q_fidelity = jsonData["two_q_fidelity"];
    for(auto [cm_str,fidelity] : two_q_fidelity)
    {
        auto [q1_id, q2_id] = parse_pair(cm_str);
        Qubit* q1 = static_cast<Qubit*>(backend->GetChild(q1_id));
        Qubit* q2 = static_cast<Qubit*>(backend->GetChild(q2_id));
        new CouplingMap(q1,q2);
    }

    //create attrib keys "T1_max", "T2_max", "q1_fidelity_max", "readout_fidelity_max", "two_q_fidelity_max"
    backend->attrib["T1_max"] = static_cast<void*>(new double());
    backend->attrib["T2_max"] = static_cast<void*>(new double());
    backend->attrib["q1_fidelity_max"] = static_cast<void*>(new double());
    backend->attrib["readout_fidelity_max"] = static_cast<void*>(new double());
    backend->attrib["two_q_fidelity_max"] = static_cast<void*>(new double());

    return 0;
}


int sys_sage::IQMParser::ParseDynamicData(int tsForHistory)
{
    double max;
    std::vector<double> T1;
    for (const auto& element : jsonData["T1"]) 
    {
        T1.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(T1.begin(), T1.end()));
    *static_cast<double*>(backend->attrib["T1_max"]) = max;

    std::vector<double> T2;
    for (const auto& element : jsonData["T2"]) 
    {
        T2.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(T2.begin(), T2.end()));
    *static_cast<double*>(backend->attrib["T2_max"]) = max;

    std::vector<double> q1_fidelity;
    for (const auto& element : jsonData["1q_fidelity"]) 
    {
        q1_fidelity.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(q1_fidelity.begin(), q1_fidelity.end()));
    *static_cast<double*>(backend->attrib["q1_fidelity_max"]) = max;

    std::vector<double> readout_fidelity;
    for (const auto& element : jsonData["readout_fidelity"]) 
    {
        readout_fidelity.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(readout_fidelity.begin(), readout_fidelity.end()));
    *static_cast<double*>(backend->attrib["readout_fidelity_max"]) = max;

    auto parse_pair = [](const std::string& pair_str) -> std::pair<int, int>
    {
        std::stringstream ss(pair_str);
        int x, y;
        char comma;
        ss >> x >> comma >> y;
        return {x, y};
    };

    for(int i=0; i<backend->GetNumQubits(); i++)
    {
        Qubit *q = dynamic_cast<Qubit*>(backend->GetChildById(i));
        q->SetProperties(T1[i], T2[i], readout_fidelity[i], q1_fidelity[i]);
        if(tsForHistory > 0)
        {
            //check if readout_history exists; if not, create it -- vector of tuples <timestamp,t1,t2,readout_fidelity,q1_fidelity>
            if(! q->attrib.contains("readout_history"))
                q->attrib["readout_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double,double,double,double>>());
            auto rh = reinterpret_cast<std::vector<std::tuple<int,double,double,double,double>>*>(q->attrib["readout_history"]);
            rh->emplace_back(tsForHistory, T1[i], T2[i], readout_fidelity[i], q1_fidelity[i]);
        }
    }
    std::map<std::string, std::string> two_q_fidelity = jsonData["two_q_fidelity"];
    max = 0;
    for(auto [cm_str,fidelity_str] : two_q_fidelity)
    {
        auto [q1_id, q2_id] = parse_pair(cm_str);
        Qubit* q1 = static_cast<Qubit*>(backend->GetChild(q1_id));
        Qubit* q2 = static_cast<Qubit*>(backend->GetChild(q2_id));
        double fidelity = std::stod(fidelity_str);
        bool cm_found = false;

        for(Relation* r_cm : q1->GetRelationsByType(sys_sage::RelationType::CouplingMap))
        {
            if(r_cm->ContainsComponent(q2))
            {
                CouplingMap* cm = reinterpret_cast<CouplingMap*>(r_cm);
                cm_found = true;
                cm->SetFidelity(fidelity);
                if(fidelity > max)
                    max = fidelity;
                        
                if(tsForHistory > 0)
                {
                    //check if readout_history exists; if not, create it -- vector of tuples <timestamp,fidelity>
                    if(! cm->attrib.contains("readout_history"))
                        cm->attrib["readout_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double>>());
                    auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(cm->attrib["readout_history"]);
                    rh->emplace_back(tsForHistory, fidelity);
                }

                break;
            }
        }
        if(!cm_found)
            return 1;        
    }
    
    *static_cast<double*>(backend->attrib["two_q_fidelity_max"]) = max;

    return 0;
}

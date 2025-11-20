#include <iomanip>
#include <algorithm>
#include "sys-sage.hpp"

using std::cout;
using std::endl;

using namespace sys_sage;

///////////////////// FoMaC functionality /////////////////////
////// This is a use-case-specific functionality, which would be a part of the sys-sage FoMaC, not the core sys-sage. hence, it is here as a separate code
#define T_WEIGHT            0.1
#define Q1_FID_WEIGHT       0.2
#define READOUT_FID_WEIGHT  0.4
#define TWO_Q_FID_WEIFHT    0.3

void calculateQubitWeight(Qubit* q, int tsForHistory = -1, double T1_max = 1, double  T2_max = 1, double  q1_fidelity_max = 1, double readout_fidelity_max = 1, double two_q_fidelity_max = 1 )
{
    if(!q->attrib.contains("qubit_weight"))
        q->attrib["qubit_weight"] = new double();

    double min_t = q->GetT1() > q->GetT2()? q->GetT2(): q->GetT1();
    double max_t_max = T1_max > T2_max ? T1_max : T2_max;

    double q_weight = min_t/max_t_max + q->Get1QFidelity()/q1_fidelity_max + q->GetReadoutFidelity()/readout_fidelity_max ;

    double coupling_map_fidelity = 0.0;
    int num_neighbours = 0;
    for(Relation* r : q->GetRelationsByType(sys_sage::RelationType::CouplingMap))
    {
        coupling_map_fidelity += (static_cast<CouplingMap*>(r))->GetFidelity();
        num_neighbours++;
    }
    q_weight += (coupling_map_fidelity/two_q_fidelity_max)/num_neighbours;

    *static_cast<double*>(q->attrib["qubit_weight"]) = q_weight;
    if(tsForHistory > 0)
    {
        //check if weight_history exists; if not, create it -- vector of tuples <timestamp,weight>
        if(! q->attrib.contains("weight_history"))
            q->attrib["weight_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double>>());
        auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(q->attrib["weight_history"]);
        rh->emplace_back(tsForHistory, q_weight);
    }

}

int calculateAllWeights(QuantumBackend* backend, int tsForHistory = -1)
{
    double T1_max = *static_cast<double*>(backend->attrib["T1_max"]);
    double T2_max = *static_cast<double*>(backend->attrib["T2_max"]);
    double q1_fidelity_max = *static_cast<double*>(backend->attrib["q1_fidelity_max"]);
    double readout_fidelity_max = *static_cast<double*>(backend->attrib["readout_fidelity_max"]);
    double two_q_fidelity_max = *static_cast<double*>(backend->attrib["two_q_fidelity_max"]);
    for(Component* c : backend->GetChildren())
    {
        if(c->GetComponentType() == sys_sage::ComponentType::Qubit)
        {
            Qubit* q = static_cast<Qubit*>(c);
            calculateQubitWeight(q, tsForHistory, T1_max, T2_max, q1_fidelity_max, readout_fidelity_max, two_q_fidelity_max );
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////

int getTs(std::string filename)
{
    size_t lastUnderscore = filename.find_last_of('_');
    size_t lastDot = filename.find_last_of('.');
    std::string result = (lastUnderscore != std::string::npos && lastDot != std::string::npos && lastUnderscore < lastDot) 
                         ? filename.substr(lastUnderscore + 1, lastDot - lastUnderscore - 1) 
                         : "";
    return std::stoi(result);
}

//calibration_data_Q-Exa_20241013.json already included in the first step
//string IQMFileNames[] = {"calibration_data_Q-Exa_20241014.json","calibration_data_Q-Exa_20241015.json","calibration_data_Q-Exa_20241016.json","calibration_data_Q-Exa_20241017.json"};
std::string IQMFileNames[] = {"calibration_data_Q-Exa_20241014.json","calibration_data_Q-Exa_20241015.json","calibration_data_Q-Exa_20241016.json","calibration_data_Q-Exa_20241017.json","calibration_data_Q-Exa_20241018.json","calibration_data_Q-Exa_20241019.json","calibration_data_Q-Exa_20241020.json","calibration_data_Q-Exa_20241021.json","calibration_data_Q-Exa_20241022.json","calibration_data_Q-Exa_20241023.json","calibration_data_Q-Exa_20241024.json","calibration_data_Q-Exa_20241025.json","calibration_data_Q-Exa_20241026.json","calibration_data_Q-Exa_20241027.json","calibration_data_Q-Exa_20241028.json","calibration_data_Q-Exa_20241029.json","calibration_data_Q-Exa_20241030.json","calibration_data_Q-Exa_20241031.json","calibration_data_Q-Exa_20241101.json","calibration_data_Q-Exa_20241102.json","calibration_data_Q-Exa_20241103.json","calibration_data_Q-Exa_20241104.json","calibration_data_Q-Exa_20241105.json","calibration_data_Q-Exa_20241106.json","calibration_data_Q-Exa_20241107.json","calibration_data_Q-Exa_20241108.json","calibration_data_Q-Exa_20241109.json","calibration_data_Q-Exa_20241110.json","calibration_data_Q-Exa_20241111.json","calibration_data_Q-Exa_20241112.json","calibration_data_Q-Exa_20241113.json","calibration_data_Q-Exa_20241114.json","calibration_data_Q-Exa_20241115.json","calibration_data_Q-Exa_20241116.json","calibration_data_Q-Exa_20241117.json","calibration_data_Q-Exa_20241118.json","calibration_data_Q-Exa_20241119.json","calibration_data_Q-Exa_20241120.json","calibration_data_Q-Exa_20241121.json","calibration_data_Q-Exa_20241122.json","calibration_data_Q-Exa_20241123.json","calibration_data_Q-Exa_20241124.json","calibration_data_Q-Exa_20241125.json","calibration_data_Q-Exa_20241126.json","calibration_data_Q-Exa_20241127.json","calibration_data_Q-Exa_20241128.json","calibration_data_Q-Exa_20241129.json","calibration_data_Q-Exa_20241130.json","calibration_data_Q-Exa_20241201.json","calibration_data_Q-Exa_20241202.json","calibration_data_Q-Exa_20241203.json","calibration_data_Q-Exa_20241204.json","calibration_data_Q-Exa_20241205.json"};

int main()
{
    std::cout << std::setprecision(15);
    QuantumBackend* b = new QuantumBackend();  
    std::string IQMPathPrefix = "/Users/stepan/phd/repos/q-sys-sage/tmp-qc-data/database/";
    std::string IQMPath;
    int IQMPathTs;
    
    IQMPath = IQMPathPrefix + "calibration_data_Q-Exa_20241013.json";
    IQMPathTs =  getTs(IQMPath);
    std::cout << "-- Parsing IQM output from file " << IQMPath << ", ts " << IQMPathTs << std::endl;
    if(parseIQM(b, IQMPath, 0, IQMPathTs) != 0) { //adds topo to a next node
        return 1;
    }
    calculateAllWeights(b,IQMPathTs);


    for(std::string file : IQMFileNames)
    {
        IQMPath = IQMPathPrefix + file;
        IQMPathTs =  getTs(IQMPath);
        std::cout << "-- Parsing IQM output from file " << IQMPath << ", ts " << IQMPathTs << std::endl;
        if(parseIQM(b, IQMPath, 0, IQMPathTs, false) != 0) { //adds topo to a next node
            return 1;
        }
        calculateAllWeights(b,IQMPathTs);
    }
    cout << "-- End parseIQM" << endl;

    // IQMPathPrefix = "/Users/stepan/phd/repos/q-sys-sage/tmp/";
    // IQMPath = IQMPathPrefix + "test_data_qexa_16.json";
    // // IQMPathTs =  getTs(IQMPath);
    // std::cout << "-- Parsing IQM output from file " << IQMPath << std::endl;
    // if(parseIQM(b, IQMPath, 0, -1) != 0) { //adds topo to a next node
    //     return 1;
    // }
    // calculateAllWeights(b,IQMPathTs);

    //SVTODO make building examples optional

    cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    b->PrintSubtree();
    
    for(Component * c : b->GetChildren() )
    {
        if(c->GetComponentType() == sys_sage::ComponentType::Qubit)
        {
            Qubit* q = static_cast<Qubit*>(c);
            std::cout << "Qubit " << q->GetId() << " has coupling map { ";
            for(Relation* r : q->GetRelationsByType(sys_sage::RelationType::CouplingMap))
            {
                CouplingMap* cm = static_cast<CouplingMap*>(r); 
                std::cout << (cm->GetComponent(0) == q ? cm->GetComponent(1)->GetId() : cm->GetComponent(0)->GetId() )<< " ";
            }
            std::cout << "} and weight = " << *static_cast<double*>(q->attrib["qubit_weight"]) << "\n";
        }
    }
    

    std::ofstream file("output.csv");
    if (!file.is_open()) 
    {
        std::cerr << "Failed to open file for writing." << std::endl;
        return 1;
    }

    file << std::fixed << std::setprecision(14);
    for(Component * c : b->GetChildren() )
    {
        if(c->attrib.contains("weight_history"))
        {
            auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(c->attrib["weight_history"]);
            for(auto entry: *rh)
                file << std::get<0>(entry) << "," << c->GetId() << "," << c->GetRelationsByType(sys_sage::RelationType::CouplingMap).size() << "," << std::get<1>(entry) << "\n";
        }
    }
    
    return 0;
}

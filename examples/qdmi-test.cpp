/* Possible exports
export CPATH=$CPATH:/home/diogenes/sys-sage/build/_deps/qinfo-src/include/
export CPATH=$CPATH:/home/diogenes/sys-sage/build/_deps/qdmi-src/include
export CONF_IBM=/home/diogenes/sys-sage/build/_deps/backends-src/tests/ibm_conf.json
export QDMI_CONFIG_FILE=/home/diogenes/sys-sage/build/_deps/qdmi-src/.qdmi-config
export PROP_IBM=/home/diogenes/sys-sage/build/_deps/backends-src/tests/ibm_prob.json
*/
#ifdef QDMI

using namespace sys_sage;

#include <iostream>
#include <string.h>
#include <iomanip>
#include "sys-sage.hpp"

int main()
{
    std::cout << std::setprecision(15);
    // Create an instance of the interface
    // To-Do: Consider QdmiParser as a static class
    QdmiParser qdmi;

    // Use QdmiParser to create the topology of either all the backends or one of the backends
    /*******************************************Method 1**********************************************/

    Topology* qc_topo = new Topology();
    qdmi.createAllQcTopo(qc_topo);

    cout << "---------------- Printing the configuration of IBM Backend----------------" << endl;
    qc_topo->PrintSubtree();
    cout << "---------------- Printing Qubit Coupling Mappings for IBM Backend----------------" << endl;
    QuantumBackend* qc = dynamic_cast<QuantumBackend*>(qc_topo->GetChild(0));
    int total_qubits = qc->CountDescendantsByType(ComponentType::Any);
    for (int i = 0; i < total_qubits; i++)
    {
        Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
        std::cout << "Qubit " << i << " has coupling map { ";
        auto coupling_map = q->GetCouplingMapping();
        for (long unsigned j = 0; j < coupling_map.size(); j++)
        {
            std::cout << coupling_map[j] << " ";
        }
        std::cout << "}\n";
    }
    cout << "---------------- Printing Supported Gate Types for IBM Backend----------------" << endl;
    auto _1q_gates = qc->FindGatesBySize(SYS_SAGE_1Q_QUANTUM_GATE);
    auto _2q_gates = qc->FindGatesBySize(SYS_SAGE_2Q_QUANTUM_GATE);
    auto _mq_gates = qc->FindGatesBySize(SYS_SAGE_MQ_QUANTUM_GATE);
    auto _0q_gates = qc->FindGatesBySize(SYS_SAGE_NO_TYPE_QUANTUM_GATE);

    if(_1q_gates.size())
    {
        int size = _1q_gates.size();
        std::cout << "Total " << size << " 1-Qubit gate(s)\n";
        for (int i = 0; i < size; ++i)
        {   
            std::cout << "Gate name:" << _1q_gates[i]->GetName() << ", ";
            std::cout << "Gate size:" << _1q_gates[i]->GetGateSize() << ", ";
            std::cout << "Gate fidelity:" << _1q_gates[i]->GetFidelity() << "\n";
        }

    }
    if(_2q_gates.size())
    {
        int size = _2q_gates.size();
        std::cout << "Total " << size << " 2-Qubit gate(s)\n";
        for (int i = 0; i < size; ++i)
        {   
            std::cout << "  Gate name:" << _2q_gates[i]->GetName() << ", ";
            std::cout << "Gate size:" << _2q_gates[i]->GetGateSize() << ", ";
            std::cout << "Gate fidelity:" << _2q_gates[i]->GetFidelity() << "\n";
        }
        
    }
    if(_mq_gates.size())
    {
        int size = _mq_gates.size();
        std::cout << "Total " << size << " M-Qubit gate(s)\n";
        for (int i = 0; i < size; ++i)
        {   
            std::cout << "  Gate name:" << _mq_gates[i]->GetName() << ", ";
            std::cout << "Gate size:" << _mq_gates[i]->GetGateSize() << ", ";
            std::cout << "Gate fidelity:" << _mq_gates[i]->GetFidelity() << "\n";
        }
        
    }
    if(_0q_gates.size())
    {
        int size = _0q_gates.size();
        std::cout << "Total " << size << " gate(s) with no type\n";
        for (int i = 0; i < size; ++i)
        {   
            std::cout << "  Gate name:" << _0q_gates[i]->GetName() << ", ";
            std::cout << "Gate size:" << _0q_gates[i]->GetGateSize() << ", ";
            std::cout << "Gate fidelity:" << _0q_gates[i]->GetFidelity() << "\n";
        }
        
    }

    std::cout << "---------------- Printing Qubit Properties for IBM Backend----------------" << endl;
    for (int i = 0; i < total_qubits; i++)
    {
        Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
        std::cout << "Qubit " << i << " has following properties: \n";
        std::cout << "      T1: " << q->GetT1() << "\n";
        std::cout << "      T2: " << q->GetT2() << "\n";
        std::cout << "      Readout Error: " << q->GetReadoutError() << "\n";
        std::cout << "      Readout Length: " << q->GetReadoutLength() << "\n";
    }

    std::cout << "---------------- Printing Coupling Maps of all the Qubits----------------" << endl;
    auto children = qc->GetAllCouplingMaps();
    for (auto it = children.begin(); it != children.end(); ++it) {
        std::cout << "{" << it->first << ", " << it->second << "}\n";
    }


    /*******************************************Method 2**********************************************/
    // auto quantum_backends = qdmi.get_available_backends();
    // std::cout << "Total " << quantum_backends.size() << " devices found.\n";
    // QuantumBackend* qc_topo = new QuantumBackend(0, "");
    // qdmi.createQcTopo(qc_topo, quantum_backends[0]);

    // cout << "---------------- Printing the configuration of QLM Backend ----------------" << endl;
    // qc_topo->PrintSubtree();
    // cout << "---------------- Printing Qubit Coupling Mappings of QLM Backend----------------" << endl;
    // int total_qubits = qc_topo->CountAllSubcomponents();
    // for (int i = 0; i < total_qubits; i++)
    // {
    //     Qubit* q = dynamic_cast<Qubit*>(qc_topo->GetChild(i));
    //     std::cout << "Qubit " << i << " has coupling map { ";
    //     auto coupling_map = q->GetCouplingMapping();
    //     for (long unsigned j = 0; j < coupling_map.size(); j++)
    //     {
    //         std::cout << coupling_map[j] << " ";
    //     }
    //     std::cout << "}\n";
    // }
       
    return 0;
}

#endif QDMI

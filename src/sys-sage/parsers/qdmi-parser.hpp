/**
* @file qdmi-parser.hpp
* @brief sys-sage's interface to QDMI.
*/

#ifndef QDMIParser_HPP
#define QDMIParser_HPP

#ifdef QDMI

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include "Component.hpp"
#include <ibm.h>

#define CHECK_ERR(a,b) { if (a!=QDMI_SUCCESS) { printf("\n[Error]: %i at %s",a,b); return 1; }}

extern "C"
{

    class QdmiParser
    {
    public:
        QdmiParser();

        /**
         * Returns all the available backends registered with QDMI.
         * @return A vector of all the backends with their names and "QDMI_Device" handles
         */
        std::vector<QDMI_Device> get_available_backends();

        int get_num_qubits(QDMI_Device dev);
        /**
         * Checks which backends are available and creates a topology with an object of type "Topology" as their parent.
         * @param topo - Object of type "Topology", which will be the parent Component of all the quntum backends. 
         */
        void createAllQcTopo(Topology *topo);

        /**
         * Checks which backends are available and creates a topology with an object of type "Topology" as their parent.
         * @returns An object of type "Topology", which will be the parent Component of all the quntum backends. 
         * @see createAllQcTopo(Topology *topo)
         */
        Topology createAllQcTopo();

        /**
         * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
         * @param backend - Object of type "QuantumBackend", storing the topology of a quantum backend.
         * @param dev - A QDMI_Device for which the topology needs to be created.
         * @see createQcTopo(Topology *topo)
         */
        void createQcTopo(QuantumBackend *backend, QDMI_Device dev);

        /**
         * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
         * @param dev - A QDMI_Device for which the topology needs to be created.
         * @param device_index - Index of the device.
         * @param device_name - Name of the device.
         * @returns Object of type "QuantumBackend", storing the topology of a quantum backend.
         * @see createQcTopo(QuantumBackend *backend, QDMI_Device dev)
         */
        QuantumBackend createQcTopo(QDMI_Device dev, int device_index = 0, std::string device_name="");

        static void refreshQubitProperties(QDMI_Device dev, Qubit *q)
        {
            //Refresh Qubit properties
            QDMI_Qubit qubits;
            int err;

            err = QDMI_query_all_qubits(dev, &qubits);

            if (err != QDMI_SUCCESS || qubits == NULL)
            {
                std::cout << "   [sys-sage]...............Could not obtain available "
                          << "qubits via QDMI\n";
                return;
            }
            int qubit_index = q->GetId();

            // Refreshing Qubit coupling map
            int coupling_map_size; 
            std::vector<int> coupling_mapping;
            coupling_map_size = (&qubits[qubit_index])->size_coupling_mapping;
            coupling_mapping.resize(coupling_map_size);
            std::copy(qubits[qubit_index].coupling_mapping, qubits[qubit_index].coupling_mapping + coupling_map_size, coupling_mapping.begin());
            q->SetCouplingMapping(coupling_mapping, coupling_map_size);


            // Set all the qubit properties
            int scope;
            // Declare prop as a vector
            std::vector<int> prop{QDMI_T1_TIME, QDMI_T2_TIME, QDMI_READOUT_ERROR, QDMI_READOUT_LENGTH};
            std::array<std::string, 4> properties{"T1", "T2", "readout_error", "readout_length"};
            double value;
            for (size_t i = 0; i < 4; ++i)
            {
                // QDMI_Qubit_property prop_index;
                QDMI_Qubit_property prop_index = new (QDMI_Qubit_property_impl_t);
                prop_index->name = prop[i];
                int err = QDMI_query_qubit_property_exists(dev, &qubits[qubit_index], prop_index, &scope);
                if(err)
                {
                    std::cout << "   [sys-sage]...............Queried property doesn't exist: " << i <<"\n";
                    continue;
                }
                err = QDMI_query_qubit_property_type(dev, &qubits[qubit_index], prop_index);
                if(prop_index->type == QDMI_DOUBLE){
                    err = QDMI_query_qubit_property_d(dev, &qubits[qubit_index], prop_index, &value);
                    if(err)
                    {
                        std::cout << "   [sys-sage]...............Unable to query property: " << i <<"\n";
                        continue;
                    }
                    //std::cout << "   [sys-sage]...............Value of " << properties[i] << ": " << value << "\n";
                }
                delete prop_index;

            }

            q->SetProperties(qubits[qubit_index].t1, qubits[qubit_index].t2, qubits[qubit_index].readout_error, qubits[qubit_index].readout_length);
        }
    private:
        int initiateSession();
        static QInfo info;
        static QDMI_Session session;
        void getCouplingMapping(QDMI_Device dev, QDMI_Qubit qubit, std::vector<int> &coupling_mapping, int &coupling_map_size);
        void getQubitProperties(QDMI_Device dev, QDMI_Qubit qubit);
        void setQubits(QuantumBackend *backend, QDMI_Device dev);
        void setGateSets(QuantumBackend *backend, QDMI_Device dev);

    };
    
}

#endif //QDMI
#endif // QDMIParser_HPP
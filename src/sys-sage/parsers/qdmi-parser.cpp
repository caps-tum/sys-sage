/**
 * @file qdmi-parser.hpp
 * @brief sys-sage's interface to QDMI.
 */

#ifdef QDMI
#include "qdmi-parser.hpp"

QInfo QdmiParser::info; 
QDMI_Session QdmiParser::session; 

extern "C" QdmiParser::QdmiParser()
{
    initiateSession();
}

extern "C" int QdmiParser::initiateSession()
{
    int err = 0;
    QdmiParser::info = NULL;
    QdmiParser::session = NULL;

    err = QInfo_create(&QdmiParser::info);
    CHECK_ERR(err, "QInfo_create");

    err = QDMI_session_init(QdmiParser::info, &QdmiParser::session);
    CHECK_ERR(err, "QDMI_session_init");
    if (err != QDMI_SUCCESS)
    {
        std::cout << "   [sys-sage]...............Unable to initiate "
                  << "QDMI session\n";
        exit(0);
    }
    std::cout << "   [sys-sage]...............Initiated "
                  << "QDMI session\n";
    return 0;    

}

extern "C" std::vector<QDMI_Device> QdmiParser::get_available_backends()
{

    int count;
    QDMI_core_device_count(&QdmiParser::session, &count);

    if(count == 0){
        std::cout << "   [sys-sage]...............QDMI_core_device_count from QDMI returned 0." << std::endl;
        return {};
    }

    else {
        std::cout << "   [sys-sage]...............QDMI_core_device_count returned " << count << ".\n";
    }

    std::vector<QDMI_Device> registered_devices;
    char **available_devices = get_qdmi_library_list_names(); // --> 
    if (available_devices == NULL)
    {
        std::cout << "   [sys-sage]...............Failed to get "
                  << "qdmi_library_list from QDMI" << std::endl;

        return {};
    }

    for(int i = 0; i < count; i++){
        QDMI_Device device;
        QDMI_core_open_device(&QdmiParser::session, i , &QdmiParser::info, &device);

        registered_devices.emplace_back(device);
    }

    return registered_devices;
}

extern "C" int QdmiParser::get_num_qubits(QDMI_Device dev)
{
    int err, num_qubits = 0;

    err = QDMI_query_qubits_num(dev, &num_qubits);
    
    if (err != QDMI_SUCCESS || num_qubits == 0)
    {
        std::cout << "   [sys-sage]...............Could not obtain number of "
                  << "available qubits via QDMI\n";
        return -1;
    }
    return num_qubits;

}

extern "C" void QdmiParser::getCouplingMapping(QDMI_Device dev, QDMI_Qubit qubit, std::vector<int> &coupling_mapping, int &coupling_map_size)
{

    if (qubit->size_coupling_mapping == 0)
    {
        std::cout << "   [sys-sage]...............No coupling mapping for qubit\n";
        return;
    }

    coupling_map_size = qubit->size_coupling_mapping;
    coupling_mapping.resize(coupling_map_size);

    std::copy(qubit->coupling_mapping, qubit->coupling_mapping + coupling_map_size, coupling_mapping.begin());
}

extern "C" void QdmiParser::getQubitProperties(QDMI_Device dev, QDMI_Qubit qubit)
{
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
        int err = QDMI_query_qubit_property_exists(dev, qubit, prop_index, &scope);
        if(err)
        {
            std::cout << "   [sys-sage]...............Queried property doesn't exist: " << i <<"\n";
            continue;
        }

        if(prop_index->type == QDMI_DOUBLE){
            err = QDMI_query_qubit_property_d(dev, qubit, prop_index, &value);
            if(err)
            {
                std::cout << "   [sys-sage]...............Unable to query property: " << i <<"\n";
                continue;
            }

        }
        delete prop_index;

    }
    
}

extern "C" void QdmiParser::setQubits(QuantumBackend *backend, QDMI_Device dev)
{
    QDMI_Qubit qubits;
    int err, num_qubits = 0;

    err = QDMI_query_all_qubits(dev, &qubits);

    if (err != QDMI_SUCCESS || qubits == NULL)
    {
        std::cout << "   [sys-sage]...............Could not obtain available "
                  << "qubits via QDMI\n";
        return;
    }

    num_qubits = backend->GetNumQubits();
    
    for (int i = 0; i < num_qubits; i++)
    {
        Qubit* q = new Qubit(backend, i);
        
        // Set coupling map
        int coupling_map_size; 
        std::vector<int> coupling_mapping;
        getCouplingMapping(dev, &qubits[i], coupling_mapping, coupling_map_size);
        q->SetCouplingMapping(coupling_mapping, coupling_map_size);

        // Set all the qubit properties
        getQubitProperties(dev, &qubits[i]);
        q->SetProperties(qubits[i].t1, qubits[i].t2, qubits[i].readout_error, qubits[i].readout_length);
    }

    free(qubits);
    return;
}

extern "C" void QdmiParser::setGateSets(QuantumBackend *backend, QDMI_Device dev)
{
    QDMI_Gate gates;
    int err, num_gates;
    
    err = QDMI_query_all_gates(dev, &gates);

    if (err != QDMI_SUCCESS || gates == NULL)
    {
        std::cout << "   [sys-sage]...............Could not obtain available "
                  << "gates via QDMI\n";
        return;
    }

    err = QDMI_query_gateset_num(dev, &num_gates);

    if (err != QDMI_SUCCESS || num_gates == 0)
    {
        std::cout << "   [sys-sage]...............Could not obtain number of "
                  << "available gates via QDMI\n";
        return;
    }
    else 
    {
        std::cout << "   [sys-sage]...............Found "
                  << num_gates << " supported gates.\n";
    }

    for (int i = 0; i < num_gates; i++)
    {

        std::string name = gates[i].name;
        double fidelity = gates[i].fidelity;
        std::string unitary = gates[i].unitary;
        size_t gate_size = gates[i].gate_size;

        // Add a constructor
        QuantumGate *qgate = new QuantumGate(gate_size);
        qgate->SetGateProperties(name, fidelity, unitary);

        backend->addGate(qgate);
    }
    
    
    free(gates);

    return;
}

extern "C" void QdmiParser::createAllQcTopo(Topology *topo)
{
    auto quantum_backends = get_available_backends();

    int total_quantum_backends = quantum_backends.size();

    for (auto i = 0; i < total_quantum_backends; ++i)
    {
        QuantumBackend* qc = new QuantumBackend(topo, i);
        createQcTopo(qc, quantum_backends[i]);
    }

}

extern "C" Topology QdmiParser::createAllQcTopo()
{

    Topology qc_topo = Topology();
    createAllQcTopo(&qc_topo);

    return qc_topo;
}

extern "C" void QdmiParser::createQcTopo(QuantumBackend *backend, QDMI_Device dev)
{
    backend->SetNumQubits(get_num_qubits(dev));
    setQubits(backend, dev);
    setGateSets(backend, dev);
}

extern "C" QuantumBackend QdmiParser::createQcTopo(QDMI_Device dev, int device_index, std::string device_name)
{
    QuantumBackend qc = QuantumBackend(device_index, device_name);
    createQcTopo(&qc, dev);

    return qc;
}

void QuantumBackend::RefreshTopology(std::set<int> qubit_indices)
{
    
    for (auto q : qubit_indices) {
        auto qubit = dynamic_cast<Qubit*>(GetChild(q));
        qubit->RefreshProperties();
    }
}

void Qubit::RefreshProperties()
{

    QuantumBackend *qc = dynamic_cast<QuantumBackend*> (this->GetParent());
    QdmiParser::refreshQubitProperties(qc->GetQDMIDevice(), this);
    
}

#endif // QDMI
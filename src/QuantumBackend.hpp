#ifndef QUANTUMBACKEND_HPP
#define QUANTUMBACKEND_HPP

#include "Component.hpp"

#include "QuantumGate.hpp"


#ifdef QDMI
#include <ibm.h>
#endif
// #include <qdmi_internal.h>

namespace sys_sage {

    /**
     * @class QuantumBackend
     * @brief Represents a quantum backend device (e.g., quantum processor or simulator).
     *
     * Inherits from Component and provides a unified abstraction for quantum hardware backends.
     * This class manages qubits, quantum gates, and their relations, and supports integration with
     * external quantum device interfaces (e.g., QDMI). It is designed to be extensible for new
     * quantum architectures and data sources.
     */
    class QuantumBackend : public Component {
    public:
        /**
        QuantumBackend constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "QuantumBackend"
        @param componentType=>SYS_SAGE_COMPONENT_QUANTUM_BACKEND
        */
        QuantumBackend(int _id = 0, std::string _name = "QuantumBackend");
        /**
        QuantumBackend constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "QuantumBackend"
        @param componentType=>SYS_SAGE_COMPONENT_QUANTUM_BACKEND
        */
        QuantumBackend(Component * parent, int _id = 0, std::string _name = "QuantumBackend");

        /**
        * @brief Sets the number of qubits in the quantum backend.
        * 
        * @param _num_qubits The number of qubits to set.
        */
        void SetNumQubits(int _num_qubits);

    #ifdef QDMI
        /**
        * @brief Sets the QDMI device for the quantum backend.
        * 
        * @param dev The QDMI device to set.
        */
        void SetQDMIDevice(QDMI_Device dev);

        /**
        * @brief Gets the QDMI device of the quantum backend.
        * 
        * @return The QDMI device.
        */
        QDMI_Device GetQDMIDevice();

        /**
         * @brief Refreshes the topology of the quantum backend.
         * 
         * @param qubit_indices The indices of the qubits that need to be refreshed.
         */
        void RefreshTopology(std::set<int> qubit_indices); // qubit_indices: indices of the qubits that need to be refreshed
    #endif
        /**
        * @brief Gets the number of qubits in the quantum backend.
        * 
        * @return The number of qubits.
        */
        int GetNumQubits () const;

        /**
        * @brief Adds a quantum gate to the quantum backend.
        * 
        * @param gate The quantum gate to add.
        */
        void addGate(QuantumGate *gate);

        /**
        * @brief Gets the quantum gates by their size.
        * 
        * @param _gate_size The size of the gates to retrieve.
        * @return A vector of quantum gates with the specified size.
        */
        [[ deprecated("Use FindGatesBySize instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<QuantumGate*> GetGatesBySize(size_t _gate_size) const;

        /**
        * @brief Gets the quantum gates by their size.
        * 
        * @param _gate_size The size of the gates to retrieve.
        * @return A vector of quantum gates with the specified size.
        */
        std::vector<QuantumGate*> FindGatesBySize(size_t _gate_size) const;

        /**
        * @brief Gets the quantum gates by their type.
        * 
        * @param _gate_type The type of the gates to retrieve.
        * @return A vector of quantum gates with the specified type.
        */
        [[ deprecated("Use FindGatesByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<QuantumGate*> GetGatesByType(QuantumGateType::type _gate_type) const;

        /**
        * @brief Gets the quantum gates by their type.
        * 
        * @param _gate_type The type of the gates to retrieve.
        * @return A vector of quantum gates with the specified type.
        */
        std::vector<QuantumGate*> FindGatesByType(QuantumGateType::type _gate_type) const;

        /**
        * @brief Gets all types of quantum gates in the quantum backend.
        * 
        * @return A vector of all quantum gate types.
        */
        std::vector<QuantumGate*> GetAllGateTypes() const;

        /**
        * @brief Gets the number of quantum gates in the quantum backend.
        * 
        * @return The number of quantum gates.
        */
        size_t GetNumberofGates() const;

        /**
        * @brief Gets all qubits in the quantum backend.
        * 
        * @return A vector of pointers to all qubits.
        */
        [[ deprecated("Use FindAllQubits instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Qubit *> GetAllQubits();

        /**
        * @brief Gets all qubits in the quantum backend.
        * 
        * @return A vector of pointers to all qubits.
        */
        std::vector<Qubit *> FindAllQubits();

        /**
        @private 
        !!Should normally not be used!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        xmlNodePtr _CreateXmlSubtree() override;

        /** Destructor for QuantumBackend. */
        ~QuantumBackend() override = default;

    private:
        int num_qubits;
        //SVTODO deal with gate_types -- can this go into Relations?
        std::vector <QuantumGate*> gate_types;
    #ifdef QDMI
        QDMI_Device device; // For refreshing the topology
    #endif
    };

}
#endif //QUANTUMBACKEND_HPP

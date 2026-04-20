#ifndef QUANTUMGATE_HPP
#define QUANTUMGATE_HPP

#include "Relation.hpp"

namespace sys_sage {

    /**
     * @class QuantumGate
     * @brief Represents a quantum gate in a quantum circuit.
     *
     * This class extends the Relation class to model quantum gates. 
     * It includes various properties specific to quantum gates, such as gate size, fidelity, and unitary matrix.
     * The class also handles the coupling map and additional properties related to quantum gate operations.
     * QuantumGate objects can represent both single- and multi-qubit gates, and are used to describe the logical operations that can be performed on a quantum backend.
     */
    class QuantumGate : public Relation {

    public:
        /**
         * @brief Constructor that initializes a quantum gate with specific properties.
         * @param _gate_size The number of qubits this gate operates on.
         * @param _name The name of the quantum gate.
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        QuantumGate(size_t _gate_size = 0, std::string _name = "", double _fidelity = 0.0, std::string _unitary = "");

        /**
         * @brief Constructor that initializes a quantum gate with a list of qubits.
         * @param _gate_size The number of qubits this gate operates on.
         * @param _qubits A vector of pointers to Qubit objects that the gate operates on.
         */
        QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits);

        /**
         * @brief Constructor that initializes a quantum gate with detailed properties and qubits.
         * @param _gate_size The number of qubits this gate operates on.
         * @param _qubits A vector of pointers to Qubit objects that the gate operates on.
         * @param _name The name of the quantum gate.
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, std::string _name, double _fidelity, std::string _unitary);

        /**
         * @brief Constructor for advanced initialization, including all properties and components.
         * @param components Vector of pointers to components (qubits) the gate acts on.
         * @param _id Optional relation ID (default 0).
         * @param _ordered Whether the gate is ordered (default true).
         * @param _gate_size The number of qubits this gate operates on.
         * @param _name The name of the quantum gate.
         * @param _gate_length The length of the gate operation (e.g., in time or circuit depth).
         * @param _gate_type The type of the quantum gate (see QuantumGateType::type).
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        QuantumGate(const std::vector<Component*>& components, int _id = 0, bool _ordered = true, size_t _gate_size = 0, std::string _name = "", int _gate_length = 0, QuantumGateType::type _gate_type = QuantumGateType::Unknown, double _fidelity = 0, std::string _unitary = "");
        /**
         * @brief Sets the properties of the quantum gate.
         * @param _name The name of the quantum gate.
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        void SetGateProperties(std::string _name, double _fidelity, std::string _unitary);

        /**
         * @brief Sets the coupling map for the quantum gate.
         * @param _coupling_mapping A vector of vectors of pointers to Qubit objects representing the coupling map.
         */
        // TODO: this needs an implementation
        void SetGateCouplingMap(std::vector<std::vector<Qubit*>> _coupling_mapping);

        /**
         * @brief Sets additional properties for the quantum gate.
         * 
         * This function allows setting properties that are not part of the core quantum gate attributes.
         */
        // TODO: this needs an implementation
        void SetAdditionalProperties();

        /**
         * @brief Sets the type of the quantum gate.
         * 
         * Sets the specific type for quantum gates.
         */
        void SetQuantumGateType();

        /**
         * @brief Gets the type of the quantum gate.
         * @return The type identifier for the quantum gate.
         */
        QuantumGateType::type GetQuantumGateType() const;

        /**
         * @brief Gets the fidelity of the quantum gate.
         * @return The fidelity of the quantum gate.
         */
        double GetFidelity() const;

        /**
         * @brief Sets the fidelity of the quantum gate.
         * @param The fidelity of the quantum gate.
         */
        void SetFidelity(double gateFidelity);

        /**
         * @brief Gets the size of the quantum gate.
         * @return The number of qubits this gate operates on.
         */
        size_t GetGateSize() const;

        /**
         * @brief Sets the size of the quantum gate.
         * @param gateSize The number of qubits this gate operates on.
         */
        void SetGateSize(size_t gateSize);

        /**
         * @brief Gets the length of the quantum gate.
         * @return The time duration or circuit depth of the quantum operation.
         */
        int GetGateLength() const;

        /**
         * @brief Sets the length of the quantum gate operation (e.g., in time or circuit depth).
         * @param GateLength The length of the gate operation.
         */
        void SetGateLength(int GateLength);

        /**
         * @brief Gets the unitary matrix of the quantum gate.
         * @return A string representing the unitary matrix of the quantum gate.
         */
        const std::string& GetUnitary() const;

        /**
         * @brief Sets the unitary matrix of the quantum gate.
         * @param gateUnitary A string representing the unitary matrix of the quantum gate.
         */
        void SetUnitary(const std::string & gateUnitary);

        /**
         * @brief Sets the name of the relationship.
         * @param _name The name of the relationship to set.
         */
        void SetName(std::string _name);

        /**
         * @brief Gets the name of the relationship.
         * @return The current name of the relationship.
         */
        std::string GetName() const;

        /**
         * @brief Prints the details of the quantum gate.
         * 
         * This method overrides the Print function in the Relation class to provide specific printing behavior for quantum gates.
         */
        void Print() const override;
        /**
         * @private
         * @brief Helper function for XML export.
         *
         * Should normally not be used directly. Used internally for exporting the quantum gate to XML.
         * @return Pointer to the created XML entry node.
         */
        xmlNodePtr _CreateXmlEntry() override;

    private:

        /**
         * @brief The size of the quantum gate, representing the number of qubits it operates on.
         */
        size_t gate_size;

        /**
         * @brief The name of the relationship.
         * 
         * This member variable stores the name or description of the relationship.
         */
        std::string name;

        /**
         * @brief The length of the quantum gate operation in terms of time or depth.
         */
        int gate_length;

        /**
         * @brief The type of the quantum gate (see QuantumGateType::type).
         */
        QuantumGateType::type gate_type;
        /**
         * @brief The fidelity of the quantum gate, indicating its accuracy or performance.
         */
        double fidelity;

        /**
         * @brief The unitary matrix representing the quantum gate's operation.
         * 
         * This string stores a serialized or symbolic representation of the unitary matrix.
         */
        std::string unitary;
    };


}
#endif //QUANTUMGATE_HPP
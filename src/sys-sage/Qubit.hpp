#ifndef QUBIT_HPP
#define QUBIT_HPP

#include "Component.hpp"

namespace sys_sage {

    /**
     * @class Qubit
     * @brief Represents a physical or logical qubit in a quantum backend.
     *
     * Inherits from Component and provides quantum-specific properties such as T1, T2, readout fidelity, and calibration time.
     * Qubit objects are used to describe the quantum resources available in a QuantumBackend and can be extended with additional
     * properties as needed for specific hardware or simulation backends.
     */
    class Qubit : public Component {
    public:
        /**
         * @brief Qubit constructor (no automatic insertion in the Component Tree).
         * @param _id Numeric ID of the qubit (default 0)
         * @param _name Name of the qubit (default "Qubit")
         *
         * Sets componentType to sys_sage::ComponentType::Qubit.
         */
        Qubit(int _id = 0, std::string _name = "Qubit");

        /**
         * @brief Qubit constructor with insertion into the Component Tree as the parent's child (as long as parent is an existing Component).
         * @param parent Pointer to the parent component
         * @param _id Numeric ID of the qubit (default 0)
         * @param _name Name of the qubit (default "Qubit")
         *
         * Sets componentType to sys_sage::ComponentType::Qubit.
         */
        Qubit(Component * parent, int _id = 0, std::string _name = "Qubit");

        /**
         * @brief Sets the properties of the qubit.
         * @param t1 The T1 relaxation time.
         * @param t2 The T2 dephasing time.
         * @param readout_fidelity The readout fidelity.
         * @param q1_fidelity 1-qubit gate fidelity (optional, default 0).
         * @param readout_length The readout length (optional, default 0).
         *
         * These properties are important for quantum error modeling and backend calibration.
         */
        void SetProperties(double t1, double t2, double readout_fidelty, double q1_fidelity = 0, double readout_length = 0);

        /**
        * @brief Gets the T1 relaxation time of the qubit.
        * @return The T1 relaxation time.
        */
        double GetT1() const;
        /**
        * @brief Gets the T2 dephasing time of the qubit.
        * @return The T2 dephasing time.
        */
        double GetT2() const;

        /**
        * @brief Gets the readout fidelity of the qubit.
        * @return The readout fidelity 
        */
        double GetReadoutFidelity() const;

        /**
        * @brief Gets the 1Q fidelity of the qubit.
        * @return 1Q fidelity 
        */
        double Get1QFidelity() const;

        /**
        * @brief Gets the readout length of the qubit.
        * @return The readout length.
        */
        double GetReadoutLength() const;

        /**
        * @brief Gets the frequency of the qubit.
        * @return The frequency of the qubit.
        */
        double GetFrequency() const;

        /**
        * @brief Gets the calibration time of the qubit.
        * @return calibration time
        */
        const std::string& GetCalibrationTime() const;

        #ifdef QDMI
        /**
         * @brief Refreshes the properties of the qubit.
         */
        void RefreshProperties();
        #endif

        /**
         * @private
         * @brief Helper function for XML dump generation.
         *
         * Should normally not be used directly. Used internally for exporting the topology to XML.
         * @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
         * @return Pointer to the created XML subtree node.
         */
        xmlNodePtr _CreateXmlSubtree() override;

        /** Destructor for Qubir. */
        ~Qubit() override = default;

    private:
        double q1_fidelity;      ///< 1Q (single-qubit gate) fidelity
        double t1;               ///< T1 relaxation time
        double t2;               ///< T2 dephasing time
        double readout_fidelity; ///< Readout fidelity
        double readout_length;   ///< Readout length
        double frequency;         ///< Qubit frequency
        std::string calibration_time; ///< Last calibration time or timestamp
    };

}
#endif //QUBIT_HPP
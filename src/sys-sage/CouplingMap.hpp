#ifndef COUPLINGMAP_HPP
#define COUPLINGMAP_HPP

#include "Relation.hpp"

namespace sys_sage {

    /**
     * @class CouplingMap
     * @brief Represents a coupling (connectivity) relation between two or more qubits/components.
     *
     * This class is used to describe the physical or logical connectivity between qubits in a quantum device,
     * typically as part of the quantum hardware abstraction in sys-sage. It inherits from Relation and can
     * represent both directed and undirected couplings, as well as additional properties such as fidelity.
     *
     * CouplingMap objects are essential for describing which qubits can interact directly (e.g., for two-qubit gates)
     * and for mapping quantum circuits to hardware. The class supports both pairwise and multi-component couplings.
     */
   class CouplingMap : public Relation {
    public:
        /**
         * @brief Constructs a CouplingMap between two qubits.
         * @param q1 Pointer to the first qubit.
         * @param q2 Pointer to the second qubit.
         *
         * This constructor is typically used for pairwise couplings in quantum hardware.
         */
        CouplingMap(Qubit* q1, Qubit*q2);

        /**
         * @private
         *
         * @brief Default-intitilizes a CouplingMap.
         * @param _id Optional relation ID (default 0).
         * @param _ordered Whether the coupling is ordered/directed (default true).
         *
         * This constructor allows for more general coupling relations, including multi-qubit couplings.
         */
        CouplingMap(int _id = 0, bool _ordered = true);

        /**
         * @brief Constructs a CouplingMap between a set of components.
         * @param components Vector of pointers to components (e.g., qubits) involved in the coupling.
         * @param _id Optional relation ID (default 0).
         * @param _ordered Whether the coupling is ordered/directed (default true).
         *
         * This constructor allows for more general coupling relations, including multi-qubit couplings.
         */
        CouplingMap(const std::vector<Component*>& components, int _id = 0, bool _ordered = true);
        /**
         * @brief Sets the fidelity value for this coupling relation.
         * @param _fidelity The fidelity (typically between 0 and 1).
         *
         * Fidelity can be used to represent the quality or reliability of the coupling (e.g., gate fidelity).
         */
        void SetFidelity(double _fidelity);
        /**
         * @brief Gets the fidelity value for this coupling relation.
         * @return The fidelity (typically between 0 and 1).
         */
        double GetFidelity() const;

        /**
         * @private
         * @brief Helper function for XML export.
         *
         * Should normally not be used directly. Used internally for exporting the coupling map to XML.
         * @return Pointer to the created XML entry node.
         */
        xmlNodePtr _CreateXmlEntry() override;

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this relation.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        void _ToJson(nlohmann::json &obj) const override;

        /**
         * @private
         *
         * @brief Initializes this relation through JSON. Intended for internal
         *        use.
         *
         * @param obj The JSON object containing the data.
         * @param componentMap A map used for the relation graph.
         *
         * @return 0 on success, 1 otherwise.
         */
        int _FromJson(const nlohmann::json &obj,
                      const std::unordered_map<uintptr_t, Component *> &componentMap) override;

    private:
        double fidelity; ///< Fidelity of the coupling (e.g., two-qubit gate fidelity)
    };
}
#endif //COUPLINGMAP_HPP
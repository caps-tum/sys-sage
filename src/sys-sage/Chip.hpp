#ifndef SYS_SAGE_CHIP_HPP
#define SYS_SAGE_CHIP_HPP

#include <sys-sage/Component.hpp>
#include <sys-sage/enums.hpp>
#include <string>

namespace sys_sage {

    /**
     * @class Chip
     * @brief Represents a building block of a node (CPU socket, GPU, NIC, etc.).
     *
     * This class is a child of Component and inherits its attributes and methods.
     * It provides a unified abstraction for various chip types, allowing for extensible
     * modeling of CPUs, GPUs, and other accelerators in heterogeneous systems.
     * The chip abstraction is designed to be flexible for new architectures and data sources.
     */
    class Chip : public Component {
    public:
        /**
         * @brief Chip constructor (no automatic insertion in the Component Tree).
         * @param _id ID of the chip (default 0)
         * @param _name Name of the chip (default "Chip")
         * @param _type Chip type (default sys_sage::ChipType::None). Defines which chip we are describing.
         *        Options: sys_sage::ChipType::None (default/generic), sys_sage::ChipType::Cpu, sys_sage::ChipType::CpuSocket, sys_sage::ChipType::Gpu.
         * @param _vendor Name of the vendor (default "")
         * @param _model Model name (default "")
         *
         * Sets componentType to sys_sage::ComponentType::Chip.
         */
        Chip(int _id = 0, std::string _name = "Chip", ChipType::type _type = ChipType::None, std::string _vendor = "", std::string _model = "");
        /**
         * @brief Chip constructor with insertion into the Component Tree as the parent's child.
         * @param parent The parent component
         * @param _id ID of the chip (default 0)
         * @param _name Name of the chip (default "Chip")
         * @param _type Chip type (default sys_sage::ChipType::None). Defines which chip we are describing.
         *        Options: sys_sage::ChipType::None (default/generic), sys_sage::ChipType::Cpu, sys_sage::ChipType::CpuSocket, sys_sage::ChipType::Gpu.
         * @param _vendor Name of the vendor (default "")
         * @param _model Model name (default "")
         *
         * Sets componentType to sys_sage::ComponentType::Chip.
         */
        Chip(Component * parent, int _id = 0, std::string _name = "Chip", ChipType::type _type = ChipType::None, std::string _vendor = "", std::string _model = "");
        /**
         * @private
         * Use Delete() or DeleteSubtree() for deleting and deallocating the components.
         */
        ~Chip() override = default;
        /**
         * @brief Sets the vendor of the chip.
         * @param _vendor The name of the vendor to set.
         */
        void SetVendor(std::string _vendor);
        /**
         * @brief Gets the vendor of the chip.
         * @return The name of the vendor.
         * @see vendor
         */
        const std::string& GetVendor() const;
        
        /**
         * @brief Sets the model of the chip.
         * @param _model The model name to set.
         */
        void SetModel(std::string _model);
        
        /**
         * @brief Gets the model of the chip.
         * @return The model name.
         * @see model
         */
        const std::string& GetModel() const;
        
        /**
         * @brief Sets the type of the chip.
         * @param chipType The chip type to set.
         */
        void SetChipType(ChipType::type chipType);
        
        /**
         * @brief Gets the type of the chip.
         * @return The chip type.
         * @see type
         */
        ChipType::type GetChipType() const;
        
        /**
         * @private
         * @brief Helper function for XML dump generation.
         *
         * Should normally not be used directly. Used internally for exporting the topology to XML.
         * @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
         * @return Pointer to the created XML subtree node.
         */
        xmlNodePtr _CreateXmlSubtree() override;
    private:
        std::string vendor; /**< Vendor of the chip */
        std::string model; /**< Model of the chip */
        ChipType::type type; /**< Type of the chip, e.g., CPU, GPU */
    #ifdef NVIDIA_MIG
    public:
        /**
         * @brief Updates the MIG settings for the chip (NVIDIA-specific).
         * @param uuid The UUID of the chip, default is an empty string.
         * @return Status of the update operation.
         */
        int UpdateMIGSettings(std::string uuid = "");

        /**
         * @brief Gets the number of SMs for the MIG.
         * @param uuid The UUID of the chip, default is an empty string.
         * @return The number of SMs.
         */
        int GetMIGNumSMs(std::string uuid = "");

        /**
         * @brief Gets the number of cores for the MIG.
         * @param uuid The UUID of the chip, default is an empty string.
         * @return The number of cores.
         */
        int GetMIGNumCores(std::string uuid = "");
    #endif
    };
}

#endif
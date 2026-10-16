#include <sys-sage/AtomSite.hpp>

sys_sage::AtomSite::AtomSite(int _id, const std::string &_name) : QuantumBackend(_id, _name, sys_sage::ComponentType::AtomSite) {}

sys_sage::AtomSite::AtomSite(Component *parent, int _id, const std::string &_name) : QuantumBackend(parent, _id, _name, sys_sage::ComponentType::AtomSite) {}

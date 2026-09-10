#include <sys-sage/attribute.hpp>

sys_sage::TypeRegistry::TypeRegistry()
{
    Register<bool>();
    Register<char>();
    Register<signed char>();
    Register<unsigned char>();
    Register<short>();
    Register<unsigned short>();
    Register<int>();
    Register<unsigned int>();
    Register<long>();
    Register<unsigned long>();
    Register<long long>();
    Register<unsigned long long>();
    Register<float>();
    Register<double>();
    Register<long double>();
    Register<std::string>();
}

sys_sage::TypeRegistry &sys_sage::TypeRegistry::Instance()
{
    static TypeRegistry registry;
    return registry;
}

std::optional<std::unique_ptr<sys_sage::IAttribute>(*)(const nlohmann::json&)> sys_sage::TypeRegistry::GetCallBack(std::string_view id) const
{
    if (auto it = callbacks.find(id); it != callbacks.end())
        return it->second;

    return std::nullopt;
}

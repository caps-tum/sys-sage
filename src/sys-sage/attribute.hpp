#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

#include <sys-sage/comptime_str_manipulation.hpp>
#include <nlohmann/json.hpp>
#include <array>
#include <concepts>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#define SYS_SAGE_PARENTHESES ()
#define SYS_SAGE_STRINGIFY(...) #__VA_ARGS__

// recursively expands a macro 128 times
#define SYS_SAGE_EXPAND_128(...) SYS_SAGE_EXPAND_64(SYS_SAGE_EXPAND_64(__VA_ARGS__))
#define SYS_SAGE_EXPAND_64(...) SYS_SAGE_EXPAND_32(SYS_SAGE_EXPAND_32(__VA_ARGS__))
#define SYS_SAGE_EXPAND_32(...) SYS_SAGE_EXPAND_16(SYS_SAGE_EXPAND_16(__VA_ARGS__))
#define SYS_SAGE_EXPAND_16(...) SYS_SAGE_EXPAND_8(SYS_SAGE_EXPAND_8(__VA_ARGS__))
#define SYS_SAGE_EXPAND_8(...) SYS_SAGE_EXPAND_4(SYS_SAGE_EXPAND_4(__VA_ARGS__))
#define SYS_SAGE_EXPAND_4(...) SYS_SAGE_EXPAND_2(SYS_SAGE_EXPAND_2(__VA_ARGS__))
#define SYS_SAGE_EXPAND_2(...) SYS_SAGE_EXPAND_1(SYS_SAGE_EXPAND_1(__VA_ARGS__))
#define SYS_SAGE_EXPAND_1(...) __VA_ARGS__

// applies the map `(x, y) -> x y` to every argument pair of the macro
// the maximum number of supported macro arguments is 128
#define SYS_SAGE_MAP_UNPACK(...) SYS_SAGE_EXPAND_128(SYS_SAGE_MAP_UNPACK_INTERNAL(__VA_ARGS__))
#define SYS_SAGE_MAP_UNPACK_INTERNAL(arg, ...) SYS_SAGE_UNPACK(arg)__VA_OPT__(, SYS_SAGE_MAP_UNPACK_INTERNAL_AGAIN SYS_SAGE_PARENTHESES (__VA_ARGS__))
#define SYS_SAGE_MAP_UNPACK_INTERNAL_AGAIN() SYS_SAGE_MAP_UNPACK_INTERNAL
#define SYS_SAGE_UNPACK(pair) SYS_SAGE_UNPACK_INTERNAL pair
#define SYS_SAGE_UNPACK_INTERNAL(x, y) x y

// applies the map `(x, y) -> y` to every argument pair of the macro
// the maximum number of supported macro arguments is 128
#define SYS_SAGE_MAP_UNPACK_SECOND(...) SYS_SAGE_EXPAND_128(SYS_SAGE_MAP_UNPACK_SECOND_INTERNAL(__VA_ARGS__))
#define SYS_SAGE_MAP_UNPACK_SECOND_INTERNAL(arg, ...) SYS_SAGE_UNPACK_SECOND(arg)__VA_OPT__(, SYS_SAGE_MAP_UNPACK_SECOND_INTERNAL_AGAIN SYS_SAGE_PARENTHESES (__VA_ARGS__))
#define SYS_SAGE_MAP_UNPACK_SECOND_INTERNAL_AGAIN() SYS_SAGE_MAP_UNPACK_SECOND_INTERNAL
#define SYS_SAGE_UNPACK_SECOND(pair) SYS_SAGE_UNPACK_SECOND_INTERNAL pair
#define SYS_SAGE_UNPACK_SECOND_INTERNAL(x, y) y

// DEVELOPER NOTE:
//     All `SYS_SAGE_REGISTER...` macros rely on a side effects to register a type at program start through static initialization of the `registered` member variable.
//     In order for this to happen, the compiler needs to emit the necessary code.
//     After a lot of tweaking (see __attribute__((used, retain)) UseRegistrar), both gcc and clang emit the code successfully.
//     However, emitting code for registering a type `T` of an attribute `Attribute<T>` is only triggered when the compiler instantiates the corresponding `TypeTrait<T>`.
//     After using a hack (see Component.inl and Relation.inl), both gcc and clang trigger instantiation of `TypeTrait<T>` whenever we insert/update/retrieve/serialize an `Attribute<T>`.
//     A type `T` is only registered once at most.
//
//     -> this functionality has been designed with a lot of nuances and intricacies in mind with respect to code generation and template instantiation performed by the compiler.
//     -> MODIFICATIONS TO THE CODE SHOULD BE DONE WITH CARE!

// DEVELOPER NOTE:
//     DO NOT CHANGE THE DEFINITION OF THE `registered` variable inside of the macros.
//     For some weird reason, clang refuses to emit the correct code if the call to the type registry is not wrapped inside of a lambda expression.

// DEVELOPER NOTE:
//     DO NOT CHANGE THE DEFINITION OF THE `UseRegistered` method inside of the macros.
//     It forces clang to emit code for the `registered` variable.

// DEVELOPER NOTE:
//     In the following macros, we have 2 versions of the `Deserialize` method each.
//     The reason is that we want to avoid nasty compilation errors when a (templated) type is specialized/registered even though it does not support deserialization.
//     This is useful in cases such as the multimap, where only a subset of multimaps is eligible for deserialization (see blacklist at the bottom).
//     If a multimap is not eligible, we want the user to still be able to insert a non-eligible multimap into the attributes map while simultaneously registering other eligible multimaps.

// Generates meta information for the specified type
// DOES NOT REGISTER THE TYPE
#define SYS_SAGE_SPECIALIZE_TYPE_TRAIT(...)                                       \
namespace sys_sage {                                                              \
    template <>                                                                   \
    struct TypeTrait<__VA_ARGS__> {                                               \
        static constexpr bool serializable = IsSerializable<__VA_ARGS__>;         \
        static constexpr bool deserializable = IsDeserializable<__VA_ARGS__>;     \
                                                                                  \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(__VA_ARGS__);     \
                                                                                  \
        template <typename U = __VA_ARGS__> requires (deserializable)             \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj) \
        {                                                                         \
            return std::make_unique<Attribute<U>>(obj.get<U>());                  \
        }                                                                         \
                                                                                  \
        template <typename U = __VA_ARGS__> requires (!deserializable)            \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)    \
        {                                                                         \
            return nullptr;                                                       \
        }                                                                         \
    };                                                                            \
}

// Generates meta information for the specified type
// REGISTERS THE TYPE
#define SYS_SAGE_REGISTER_TYPE_TRAIT(...)                                             \
namespace sys_sage {                                                                  \
    template <>                                                                       \
    struct TypeTrait<__VA_ARGS__> {                                                   \
        static constexpr bool serializable = IsSerializable<__VA_ARGS__>;             \
        static constexpr bool deserializable = IsDeserializable<__VA_ARGS__>;         \
                                                                                      \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(__VA_ARGS__);         \
                                                                                      \
        template <typename U = __VA_ARGS__> requires (deserializable)                 \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)     \
        {                                                                             \
            return std::make_unique<Attribute<U>>(obj.get<U>());                      \
        }                                                                             \
                                                                                      \
        template <typename U = __VA_ARGS__> requires (!deserializable)                \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)        \
        {                                                                             \
            return nullptr;                                                           \
        }                                                                             \
                                                                                      \
        template <typename U = __VA_ARGS__>                                           \
        inline static const bool registered = []                                      \
        {                                                                             \
            return TypeRegistry::Instance().Register<U>();                            \
        }();                                                                          \
                                                                                      \
        __attribute__((used, retain))                                                 \
        static void UseRegistered()                                                   \
        {                                                                             \
            if constexpr (serializable && deserializable)                             \
                (void) registered<__VA_ARGS__>;                                       \
        }                                                                             \
    };                                                                                \
}

// Generates meta information for the specified templated type
// DOES NOT REGISTER THE TEMPLATED TYPE
#define SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(type)                                                                              \
namespace sys_sage {                                                                                                                \
    template <typename... Ts>                                                                                                       \
    struct TypeTrait<type<Ts...>> {                                                                                                 \
        static constexpr bool serializable = IsSerializable<type<Ts...>>;                                                           \
        static constexpr bool deserializable = IsDeserializable<type<Ts...>>;                                                       \
                                                                                                                                    \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(type), "<", CompStrCatVariadicTemplate<Ts...>(), ">">(); \
                                                                                                                                    \
        template <typename U = type<Ts...>> requires (deserializable)                                                               \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                   \
        {                                                                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                    \
        }                                                                                                                           \
                                                                                                                                    \
        template <typename U = type<Ts...>> requires (!deserializable)                                                              \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)                                                      \
        {                                                                                                                           \
            return nullptr;                                                                                                         \
        }                                                                                                                           \
    };                                                                                                                              \
}

// Generates meta information for the specified templated type
// ONLY REGISTERS A FULLY INSTANTIATED TEMPLATED TYPE
#define SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(type)                                                                                \
namespace sys_sage {                                                                                                                \
    template <typename... Ts>                                                                                                       \
    struct TypeTrait<type<Ts...>> {                                                                                                 \
        static constexpr bool serializable = IsSerializable<type<Ts...>>;                                                           \
        static constexpr bool deserializable = IsDeserializable<type<Ts...>>;                                                       \
                                                                                                                                    \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(type), "<", CompStrCatVariadicTemplate<Ts...>(), ">">(); \
                                                                                                                                    \
        template <typename U = type<Ts...>> requires (deserializable)                                                               \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                   \
        {                                                                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                    \
        }                                                                                                                           \
                                                                                                                                    \
        template <typename U = type<Ts...>> requires (!deserializable)                                                              \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)                                                      \
        {                                                                                                                           \
            return nullptr;                                                                                                         \
        }                                                                                                                           \
                                                                                                                                    \
        template <typename U = type<Ts...>>                                                                                         \
        inline static const bool registered = []                                                                                    \
        {                                                                                                                           \
            return TypeRegistry::Instance().Register<U>();                                                                          \
        }();                                                                                                                        \
                                                                                                                                    \
        __attribute__((used, retain))                                                                                               \
        static void UseRegistered()                                                                                                 \
        {                                                                                                                           \
            if constexpr (serializable && deserializable)                                                                           \
                (void) registered<type<Ts...>>;                                                                                     \
        }                                                                                                                           \
    };                                                                                                                              \
}

// Generates COMPILER-SPECIFIC meta information for the specified type
// DOES NOT REGISTER THE TYPE
#define SYS_SAGE_SPECIALIZE_TYPE_TRAIT_NON_PORTABLE(...)                          \
namespace sys_sage {                                                              \
    template <>                                                                   \
    struct TypeTrait<__VA_ARGS__> {                                               \
        static constexpr bool serializable = IsSerializable<__VA_ARGS__>;         \
        static constexpr bool deserializable = IsDeserializable<__VA_ARGS__>;     \
                                                                                  \
        static constexpr decltype(auto) id = CompStrExtract<__VA_ARGS__>();       \
                                                                                  \
        template <typename U = __VA_ARGS__> requires (deserializable)             \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj) \
        {                                                                         \
            return std::make_unique<Attribute<U>>(obj.get<U>());                  \
        }                                                                         \
                                                                                  \
        template <typename U = __VA_ARGS__> requires (!deserializable)            \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)    \
        {                                                                         \
            return nullptr;                                                       \
        }                                                                         \
    };                                                                            \
}

// Generates COMPILER-SPECIFIC meta information for the specified type
// REGISTERS THE TYPE
#define SYS_SAGE_REGISTER_TYPE_TRAIT_NON_PORTABLE(...)                                \
namespace sys_sage {                                                                  \
    template <>                                                                       \
    struct TypeTrait<__VA_ARGS__> {                                                   \
        static constexpr bool serializable = IsSerializable<__VA_ARGS__>;             \
        static constexpr bool deserializable = IsDeserializable<__VA_ARGS__>;         \
                                                                                      \
        static constexpr decltype(auto) id = CompStrExtract<__VA_ARGS__>();           \
                                                                                      \
        template <typename U = __VA_ARGS__> requires (deserializable)                 \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)     \
        {                                                                             \
            return std::make_unique<Attribute<U>>(obj.get<U>());                      \
        }                                                                             \
                                                                                      \
        template <typename U = __VA_ARGS__> requires (!deserializable)                \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)        \
        {                                                                             \
            return nullptr;                                                           \
        }                                                                             \
                                                                                      \
        template <typename U = __VA_ARGS__>                                           \
        inline static const bool registered = []                                      \
        {                                                                             \
            return TypeRegistry::Instance().Register<U>();                            \
        }();                                                                          \
                                                                                      \
        __attribute__((used, retain))                                                 \
        static void UseRegistered()                                                   \
        {                                                                             \
            if constexpr (serializable && deserializable)                             \
                (void) registered<__VA_ARGS__>;                                       \
        }                                                                             \
    };                                                                                \
}

// Generates COMPILER-SPECIFIC meta information for the specified templated type
// DOES NOT REGISTER THE TEMPLATED TYPE
#define SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT_NON_PORTABLE(type, ...)                                        \
namespace sys_sage {                                                                                            \
    template <SYS_SAGE_MAP_UNPACK(__VA_ARGS__)>                                                                 \
    struct TypeTrait<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>> {                                           \
        static constexpr bool serializable = IsSerializable<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>;     \
        static constexpr bool deserializable = IsDeserializable<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>; \
                                                                                                                \
        static constexpr decltype(auto) id = CompStrExtract<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>();   \
                                                                                                                \
        template <typename U = type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>> requires (deserializable)         \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                               \
        {                                                                                                       \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                \
        }                                                                                                       \
                                                                                                                \
        template <typename U = type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>> requires (!deserializable)        \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)                                  \
        {                                                                                                       \
            return nullptr;                                                                                     \
        }                                                                                                       \
    };                                                                                                          \
}

// Generates COMPILER-SPECIFIC meta information for the specified templated type
// ONLY REGISTERS A FULLY INSTANTIATED TEMPLATED TYPE
#define SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT_NON_PORTABLE(type, ...)                                          \
namespace sys_sage {                                                                                            \
    template <SYS_SAGE_MAP_UNPACK(__VA_ARGS__)>                                                                 \
    struct TypeTrait<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>> {                                           \
        static constexpr bool serializable = IsSerializable<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>;     \
        static constexpr bool deserializable = IsDeserializable<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>; \
                                                                                                                \
        static constexpr decltype(auto) id = CompStrExtract<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>();   \
                                                                                                                \
        template <typename U = type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>> requires (deserializable)         \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                               \
        {                                                                                                       \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                \
        }                                                                                                       \
                                                                                                                \
                                                                                                                \
        template <typename U = type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>> requires (!deserializable)        \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)                                  \
        {                                                                                                       \
            return nullptr;                                                                                     \
        }                                                                                                       \
                                                                                                                \
        template <typename U = type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>                                   \
        inline static const bool registered = []                                                                \
        {                                                                                                       \
            return TypeRegistry::Instance().Register<U>();                                                      \
        }();                                                                                                    \
                                                                                                                \
        __attribute__((used, retain))                                                                           \
        static void UseRegistered()                                                                             \
        {                                                                                                       \
            if constexpr (serializable && deserializable)                                                       \
                (void) registered<type<SYS_SAGE_MAP_UNPACK_SECOND(__VA_ARGS__)>>;                               \
        }                                                                                                       \
    };                                                                                                          \
}

// blacklists the specified type from JSON serialization
#define SYS_SAGE_BLACKLIST_TYPE_FROM_SERIALIZATION(...)                     \
namespace sys_sage {                                                        \
    template <>                                                             \
    struct IsBlacklistedFromSerialization<__VA_ARGS__> : std::true_type {}; \
}

// blacklists the specified templated type from JSON serialization
#define SYS_SAGE_BLACKLIST_TEMPLATED_TYPE_FROM_SERIALIZATION(type)          \
namespace sys_sage {                                                        \
    template <typename... Ts>                                               \
    struct IsBlacklistedFromSerialization<type<Ts...>> : std::true_type {}; \
}

// blacklists the specified type from JSON deserialization
#define SYS_SAGE_BLACKLIST_TYPE_FROM_DESERIALIZATION(...)                     \
namespace sys_sage {                                                          \
    template <>                                                               \
    struct IsBlacklistedFromDeserialization<__VA_ARGS__> : std::true_type {}; \
}

// blacklists the specified templated type from JSON deserialization
#define SYS_SAGE_BLACKLIST_TEMPLATED_TYPE_FROM_DESERIALIZATION(type)          \
namespace sys_sage {                                                          \
    template <typename... Ts>                                                 \
    struct IsBlacklistedFromDeserialization<type<Ts...>> : std::true_type {}; \
}

namespace sys_sage {
    /**
     * @brief A concept for checking if a `to_json` function exists for the given type.
     */
    template <typename T>
    concept HasToJson = requires (nlohmann::json &obj, const T &attr) { nlohmann::adl_serializer<T>::to_json(obj, attr); };

    /**
     * @brief A concept for checking if a `from_json` function exists for the given type.
     */
    template <typename T>
    concept HasFromJson = requires (const nlohmann::json &obj, T &attr) { nlohmann::adl_serializer<T>::from_json(obj, attr); }
                       || requires (const nlohmann::json &obj) { { nlohmann::adl_serializer<T>::from_json(obj) } -> std::same_as<T>; };

    /**
     * @brief Blacklists types that do not support JSON serialization, even though a `to_json` function exists.
     *        It is used to circumvent a bug in nlohmann-json.
     */
    template <typename T>
    struct IsBlacklistedFromSerialization : std::false_type {};

    /**
     * @brief Blacklists types that do not support JSON deserialization, even though a `from_json` function exists.
     *        It is used to circumvent a bug in nlohmann-json.
     */
    template <typename T>
    struct IsBlacklistedFromDeserialization : std::false_type {};

    /**
     * @brief A concept for checking if a type supports JSON serialization.
     */
    template <typename T>
    concept IsSerializable = !IsBlacklistedFromSerialization<T>::value && HasToJson<T>;

    /**
     * @brief A concept for checking if a type supports JSON deserialization.
     */
    template <typename T>
    concept IsDeserializable = !IsBlacklistedFromDeserialization<T>::value && HasFromJson<T>;

    /**
     * @class IAttribute
     *
     * @brief A polymorphic interface for type-erased attributes.
     */
    class IAttribute {
    public:
        /**
         * @brief Virtual destructor for correct cleanup of attributes.
         */
        virtual ~IAttribute() = default;

        /**
         * @brief Virtual JSON serialization for atttributes.
         *
         * @param obj The JSON object to be initialized to represent this attribute.
         */
        virtual void Serialize(nlohmann::json &obj) const = 0;
    };

    // forward declaration
    template <typename T>
    struct TypeTrait;
    
    /**
     * @class Attribute
     *
     * @brief A wrapper that stores an attribute of any type.
     */
    template <typename T>
    class Attribute : public IAttribute {
    public:
        /**
         * @brief Generic constructor using perfect forwarding.
         *
         * @param value Initializes this attribute using the given value.
         */
        template <typename U>
        Attribute(U &&_value);
    
        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        attribute.
         *
         * @return A reference to the value.
         */
        T &operator*();

        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        constant attribute.
         *
         * @return A reference to the const value.
         */
        const T &operator*() const;
    
        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        attribute.
         *
         * @return A pointer to the value.
         */
        T *operator->();

        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        constant attribute.
         *
         * @return A pointer to the constant value.
         */
        const T *operator->() const;

        /**
         * @brief JSON serialization for atttributes.
         *
         * @param obj The JSON object to be initialized to represent this attribute.
         */
        void Serialize(nlohmann::json &obj) const override;
    
    private:
        /**
         * @brief The underlying value.
         */
        T value;
    };

    /**
     * @class TypeRegistry
     *
     * @brief Manages type registration and stores deserialization callbacks functions.
     */
    class TypeRegistry {
    public:
        /**
         * @brief Returns the global static type registry.
         */
        static TypeRegistry &Instance();
    
        /**
         * @brief Registers a type.
         *
         * @return Returns `true` if registration succeeded, `false` otherwise.
         */
        template <typename T>
        bool Register()
        {
            if constexpr (TypeTrait<T>::serializable && TypeTrait<T>::deserializable) {
                callbacks.emplace(TypeTrait<T>::id, TypeTrait<T>::template Deserialize<>);
                return true;
            } else {
                return false;
            }
        }
    
        /**
         * @brief Returns the deserialization callback associated to this type.
         *
         * @param id A unique identifier for the `TypeDescriptor`.
         */
        std::optional<std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> GetCallBack(std::string_view id) const;
    
    private:
        /**
         * @brief Initializes the registry with some pre-defined types.
         */
        TypeRegistry();
        /**
         * @brief Maps the unique identifier of a `TypeDescriptor` to its deserialization callback routine.
         */
        std::unordered_map<std::string_view, std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> callbacks;
    };

    /**
     * @class TypeTraits
     *
     * @brief Provides compile time meta data for a type.
     */
    template <typename T>
    struct TypeTrait {
        static constexpr bool serializable = IsSerializable<T>;
        static constexpr bool deserializable = IsDeserializable<T>;

        static constexpr decltype(auto) id = CompStrExtract<T>();
    };

    template <CompStr compStr>
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return CompStrToLiteral<compStr>();
    }

    template <CompStr compStr, typename T, typename... Ts>
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return CompStrCatVariadicTemplate<CompStrCat<compStr, ", ", TypeTrait<T>::id>(), Ts...>();
    }

    template <typename T, typename... Ts>
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return CompStrCatVariadicTemplate<TypeTrait<T>::id, Ts...>();
    }

    template <typename... Ts> requires (sizeof...(Ts) == 0)
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return "";
    }
}

///////////////////////////////////////////////////////////////////////////////
////// SOME OUT-OF-THE-BOX TYPE TRAIT SPECIALIZATIONS AND REGISTRATIONS ///////
///////////////////////////////////////////////////////////////////////////////

SYS_SAGE_SPECIALIZE_TYPE_TRAIT(bool)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(char)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(signed char)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned char)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(short)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned short)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(int)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned int)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(long long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned long long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(float)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(double)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(long double)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(std::string)

SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::vector)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::map)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::unordered_map)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::tuple)

SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::allocator)
SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::less)
SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::hash)
SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::equal_to)

namespace sys_sage {
    template <typename T1, typename T2>
    struct TypeTrait<std::pair<const T1, T2>> {
        static constexpr bool serializable = IsSerializable<std::pair<const T1, T2>>;
        static constexpr bool deserializable = IsDeserializable<std::pair<const T1, T2>>;

        static constexpr decltype(auto) id = CompStrCat<"std::pair<const ", TypeTrait<T1>::id, ", ", TypeTrait<T2>::id, ">">();

        template <typename U = std::pair<const T1, T2>> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<U>>(obj.get<U>());
        }

        template <typename U = std::pair<const T1, T2>> requires (!deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &)
        {
            return nullptr;
        }
    };
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// BLACKLIST SOME TYPES //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace sys_sage {
    template <typename Key, typename T, typename Compare, typename Allocator>
    struct IsBlacklistedFromDeserialization<std::multimap<Key, T, Compare, Allocator>> : std::bool_constant<!std::same_as<Key, std::string>> {};
}

namespace sys_sage {
    template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
    struct IsBlacklistedFromDeserialization<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>> : std::bool_constant<!std::same_as<Key, std::string>> {};
}

#include <sys-sage/attribute.inl>

#endif

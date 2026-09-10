# JSON Serialization & Deserialization

_sys-sage_ provides the means to dump the entire component tree and its associated relations to JSON.
In addition, the component tree can be loaded back from JSON.
This way, a snapshot of the dynamic state of the hardware topology can be saved, inspected and reconstructed.
All functionality has been successfully tested on gcc (>=12.1) and clang (>=16.0.0).
The relevant functions in the library's API are:

```cpp
sys_sage::DumpJson
sys_sage::LoadJson
```

An example is shown below

```cpp
#include <sys-sage.hpp>
#include <iostream>
#include <string>

namespace ss = sys_sage;

int main()
{
    ss::Component *l2cache0 = new ss::Cache(0, 2);
    ss::Component *l1cache0 = new ss::Cache(l2cache0, 0, 1);
    ss::Component *l1cache1 = new ss::Cache(l2cache0, 1, 1);
    ss::Component *core0 = new ss::Core(l1cache0, 0);
    ss::Component *core1 = new ss::Core(l1cache1, 1);

    l2cache0->SetAttribute<std::string>("write policy", "write-back/write-allocate");

    ss::Relation *datapathCore0L2Cache0 = new ss::DataPath(core0, l2cache0, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 199e9, 2.5e-9);
    ss::Relation *datapathCore1L2Cache0 = new ss::DataPath(core1, l2cache0, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 200e9, 2.4e-9);
    ss::Relation *datapathCore0L1Cache0 = new ss::DataPath(core0, l1cache0, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 401e9, 0.4e-9);
    ss::Relation *datapathCore1L1Cache1 = new ss::DataPath(core1, l1cache1, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 399e9, 0.6e-9);

    ss::DumpJson(l2cache0, "topo.json");

    ss::Component::DeleteSubtree(l2cache0);

    return 0;
}
```

with corresponding output

```
{
    "componentTree": {
        "address": 94005482936752,
        "attributes": {
            "write policy": {
                "_sys_sage_type": "std::string",
                "_sys_sage_value": "write-back/write-allocate"
            }
        },
        "cacheType": "2",
        "children": [
            {
                "address": 94005482936960,
                "cacheType": "1",
                "children": [
                    {
                        "address": 94005482937440,
                        "id": 0,
                        "type": "Core"
                    }
                ],
                "id": 0,
                "type": "Cache"
            },
            {
                "address": 94005482937200,
                "cacheType": "1",
                "children": [
                    {
                        "address": 94005482937600,
                        "id": 1,
                        "type": "Core"
                    }
                ],
                "id": 1,
                "type": "Cache"
            }
        ],
        "id": 0,
        "type": "Cache"
    },
    "relationGraph": [
        {
            "bandwidth": 200000000000.0,
            "category": 0,
            "components": [
                94005482937600,
                94005482936752
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 2.4e-09,
            "ordered": false,
            "type": "DataPath"
        },
        {
            "bandwidth": 399000000000.0,
            "category": 0,
            "components": [
                94005482937600,
                94005482937200
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 6e-10,
            "ordered": false,
            "type": "DataPath"
        },
        {
            "bandwidth": 199000000000.0,
            "category": 0,
            "components": [
                94005482937440,
                94005482936752
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 2.5e-09,
            "ordered": false,
            "type": "DataPath"
        },
        {
            "bandwidth": 401000000000.0,
            "category": 0,
            "components": [
                94005482937440,
                94005482936960
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 4e-10,
            "ordered": false,
            "type": "DataPath"
        }
    ]
}
```

## Attributes

Since a component's attribute can be of arbitrary type, _sys-sage_ maintains a type registry system to automatically dump and load arbitrary attributes.
In order for _sys-sage_ to know how to process an attribute, its type needs to be registered and serialization/deserialization callbacks need to be provided.

### Table of Contents

1. [Serialization & Deserialization Callbacks](#serialization--deserialization-callbacks)
2. [Standard Type Specialization & Registration](#standard-type-specialization--registration)
3. [Pre-registered Types](#pre-registered-types)
4. [Important Details about Implicit Registration and Macro Usage](#important-details-about-implicit-registration-and-macro-usage)
5. [Meta Information](#meta-information)
6. [False Positives & Blacklisted Types](#false-positives--blacklisted-types)
7. [Non-typed Template Arguments](#non-typed-template-arguments)
8. [Python Bindings](#python-bindings)
9. [Generating Meta Data for Custom Type Specializations & Registrations](#generating-meta-data-for-custom-type-specializations--registrations)

### Serialization & Deserialization Callbacks

_sys-sage_ relies on the callbacks used natively by the `nlohmann-json` library, i.e. a type must provide a `to_json` function for serialization and a `from_json` function for deserialization.
The documentation for arbitrary type conversion can be found [here](https://json.nlohmann.me/features/arbitrary_types/).
The following illustrates an example:

```cpp
struct Foo
{
    int x;
    int y;
};

// enable serialization
void to_json(nlohmann::json &obj, const Foo &foo)
{
    obj["x"] = foo.x;
    obj["y"] = foo.y;
}

// enable deserialization
void from_json(const nlohmann::json &obj, Foo &foo)
{
    obj.at("x").get_to(foo.x);
    obj.at("y").get_to(foo.y);
}
```

### Standard Type Specialization & Registration

Per default, an attribute whose type is serializable, i.e. a corresponding `to_json` function exists, will be serialized automatically.
However, deserialization only happens when the type is registered.
The _sys-sage_ library provides the following macros for standard type registry:

| macros for implicit type registry |
| --------------------------------- |
| SYS_SAGE_REGISTER_TYPE_TRAIT |
| SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT |

Both generate some meta information that is used to register a type **implicitly** at **program start** through static initialization.
Note that registration only happens if the type is serializable and deserializable.
Otherwise it's ignored.

The latter macro is intended for generating meta information for templated types like `std::vector<T>`, such that one only has to specify it once and _sys-sage_ knows how to register `std::vector<int>`, `std::vector<std::string>` or any other `T`.
Alternatively, one can register a fully instantiated templated type directly.
Registration is not limited to the number of template arguments, meaning that we also support variadic template arguments.
Moreover, we do not distinguish between templated arguments with or without default values.
The `SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT` macro should only be used for types that only take **typed template arguments**.
Refer to the sections below for non-typed template arguments.
Some basic usage is shown here:

```cpp
// let's say there exists a struct `Foo` and a class `Bar` inside the namespace `FooBar`
namespace FooBar
{
    struct Foo;

    template <typename T1, typename T2>
    class Bar;
}

// register type `FooBar::Foo`
SYS_SAGE_REGISTER_TYPE_TRAIT(FooBar::Foo)

// register templated type `FooBar::Bar<T1, T2>`
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(FooBar::Bar)

// register fully instantiated templated type `FooBar::Bar<int, int>`
SYS_SAGE_REGISTER_TYPE_TRAIT(FooBar::Bar<int, int>)
```

As you can see, the macro needs to be used in the **global namespace** and the full namespace of the type needs to be specified.
Moreover, the explicit registration of a fully instantiated templated type has higher precedence over the more general registration of the templated type.
This means that the compiler would use the meta data generated by `SYS_SAGE_REGISTER_TYPE_TRAIT(FooBar::Bar<int, int>)` over `SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(FooBar::Bar)` for every attribute of type `FooBar::Bar<int, int>`.
In this example it wouldn't make any practical difference, but this behavior may be exploited to generate custom meta data for a type without using the macro to override the more general definition for more custom behavior.
More on that later.

One can also register (templated) types **explicitly**.
This may be helpful for debugging, since it avoids side effects during program start, and to (marginally) reduce static memory usage.
Nevertheless, some meta data still needs to be generated regardless by using the macros below:

| macros for explicit type registry |
| --------------------------------- |
| SYS_SAGE_SPECIALIZE_TYPE_TRAIT |
| SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT |

They are drop-in replacements for the ones before.
To register a type `T`, one then needs to call the function `sys_sage::TypeRegistry::Instance().Register<T>()` at runtime.
Templated types need to be registered for every type argument, e.g. `sys_sage::TypeRegistry::Instance().Register<std::vector<int>>()` and `sys_sage::TypeRegistry::Instance().Register<std::vector<std::string>>()`.
Note that this function needs to be called before loading from JSON.
We recommend to simply call this function as part of an initialization routine at the very beginning of the main function.

### Pre-registered Types

The _sys-sage_ library already comes with some pre-registered types **out-of-the-box**.
We hope that it covers most of the basic usage, such that users only have to register user-defined types.
Pre-registered types include

| pre-registered (templated) types |
| -------------------------------- |
| bool |
| char |
| signed char |
| unsigned char |
| short |
| unsigned short |
| int |
| unsigned int |
| long |
| unsigned long |
| long long |
| unsigned long long |
| float |
| double |
| long double |
| std::string |
| std::vector |
| std::map |
| std::unordered_map |
| std::tuple |

Some "helper" templated types have been "specialized" using the `SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT` macro without being registered:

| "pre-specialized" templated types |
| --------------------------------- |
| std::allocator |
| std::less |
| std::hash |
| std::equal_to |
| std::pair<const T1, T2> |

Note that we have only specialized `std::pair` where the first template argument is `const`.

### Important Details about Implicit Registration and Macro Usage

There exist some intricacies when it comes to implict registration of templated types.
Consider the following example: Assume the file `topo.json` has the following content

```
{
    "componentTree": {
        "address": 94132193966656,
        "attributes": {
            "bar": {
                "_sys_sage_type": "FooBar::Bar<int, double>",
                "_sys_sage_value": {
                    "x": 1,
                    "y": 1.5
                }
            }
        },
        "id": 0,
        "type": "GenericComponent"
    }
}
```

which was generated by this code:

```cpp
#include <sys-sage.hpp>
#include <nlohmann/json.hpp>

namespace FooBar
{
    template <typename T1, typename T2>
    class Bar
    {
    public:
        T1 x;
        T2 y;
    };

    template <typename T1, typename T2>
    void to_json(nlohmann::json &obj, const Bar<T1, T2> &bar)
    {
        obj["x"] = bar.x;
        obj["y"] = bar.y;
    }
    
    template <typename T1, typename T2>
    void from_json(const nlohmann::json &obj, Bar<T1, T2> &bar)
    {
        obj.at("x").get_to(bar.x);
        obj.at("y").get_to(bar.y);
    }
}

SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(FooBar::Bar)

int main()
{
    sys_sage::Component *comp = new sys_sage::Component;
    comp->SetAttribute("bar", FooBar::Bar<int, double>{1, 1.5});
    sys_sage::DumpJson(comp, "topo.json");
    delete comp;

    return 0;
}
```

We clearly rely on implicit registration of `FooBar::Bar<T1, T2>` for any `T1` and `T2`.
If we now load the `topo.json` and print the number of attributes like so:

```cpp
#include <sys-sage.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

namespace FooBar
{
    template <typename T1, typename T2>
    class Bar
    {
    public:
        T1 x;
        T2 y;
    };

    template <typename T1, typename T2>
    void to_json(nlohmann::json &obj, const Bar<T1, T2> &bar)
    {
        obj["x"] = bar.x;
        obj["y"] = bar.y;
    }
    
    template <typename T1, typename T2>
    void from_json(const nlohmann::json &obj, Bar<T1, T2> &bar)
    {
        obj.at("x").get_to(bar.x);
        obj.at("y").get_to(bar.y);
    }
}

SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(FooBar::Bar)

int main()
{
    sys_sage::Component *comp = sys_sage::LoadJson("topo.json");
    std::cout << comp->GetAttributesSize() << '\n';
    delete comp;

    return 0;
}
```

the output will be `0`.
As mentioned before, `SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(FooBar::Bar)` will only generate meta information such that _sys-sage_ knows how to register a `FooBar::Bar<T1, T2>`.
Implicit registration of a particual instance of the templated type, e.g. `FooBar::Bar<int, double>` with `T1 = int` and `T2 = double`, only happens if that instance was **referenced** in any of the attributes.
By "referencing a type through an attribute" we mean inserting/updating/retrieving an attribute of that type.
Note that referencing a type just once is sufficient enough to trigger registration, thus all attributes of that type will be deserialized.
You don't have to reference every single attribute for it to be deserialized.

Since the above example never uses an attribute of type `FooBar::Bar<int, double>`, it will not be registered, hence the attribute from the `topo.json` file will not be deserialized.
This behavior is valid, because we don't have to deserialize an attribute if it is never used.
If you were to use the attribute somewhere, e.g. `comp->GetAttribute<FooBar::Bar<int, double>>("bar")`, or rely on explicit registration or directly registered the particular instance via `SYS_SAGE_REGISTER_TYPE_TRAIT(FooBar::Bar<int, double>)` instead, deserialization would occur.

Another important aspect is where to put the macros.
In a nutshell, we recommend placing the macros used for specializing/registering a (templated) type in the file in which the (templated) type is defined. e.g. the header file.
If this is not possible, then create a new header file that contains all specialization/registration macros and include that header whenever you insert/update/retrieve/serialize/deserialize an attribute of that type.

### Meta Information

One can retrieve meta information about a type `T` of an attribute by using fields of the `sys_sage::TypeTrait<T>` struct.
The relevant fields are

| struct field | meta information |
| ------------ | ---------------- |
| `sys_sage::TypeTrait<T>::serializable` | A bool that indicates whether `T` is eligible for serialization |
| `sys_sage::TypeTrait<T>::deserializable` | A bool that indicates whether `T` is eligible for deserialization |
| `sys_sage::TypeTrait<T>::id` | A unique string literal representing `T` that is used for distinguishing types during deserialization |
| `sys_sage::TypeTrait<T>::registered` | A bool that indicates whether `T` has been successfully registered (only valid for types that are registered with the `SYS_SAGE_REGISTER...` macros)  |

The first three fields are `constexpr`.
Use these fields for debugging purposes.

### False Positives & Blacklisted Types

Both `sys_sage::TypeTrait<T>::serializable` and `sys_sage::TypeTrait<T>::deserializable` can sometimes yield false positives.
This can result in a huge compilation errors in which the meta data generated for a specialized/register (templated) type indicates that it can be serialized/deserialized, but the compiler fails to generate the corresponding code for serialization/deserialization due to some internal implementation within `nlohmann-json`.
The only workaround we found was to explicitly blacklist some (templated) types either from serialization or deserialization.
_sys-sage_ has blacklisted the following types

| blacklisted (templated) types | blacklisted from |
| ----------------------------- | ---------------- |
| `std::multimap` where the key is **not** an `std::string` | deserialization |
| `std::unordered_multimap` where the key is **not** an `std::string` | deserialization |

If you encounter any such compilation errors when specializing/registering a type `T`, try to identify the false positive.
To check whether serialization of `T` is the problem, try to compile

```cpp
if constexpr (sys_sage::TypeTrait<T>::serializable) {
    T t;
    nlohmann::json obj = t;
}
```

If this doesn't compile, try to blacklist `T` from serialization and check if the compilation error still persists (or you might as well not register `T`).
For deserialization, try to compile

```cpp
T t1;
nlohmann::json obj = t1;
if constexpr (sys_sage::TypeTrait<T>::deserializable) {
    T t2 = obj.get<T>();
}
```

To blacklist a (templated) type, use the macros

| macros for blacklisting |
| ----------------------- |
| SYS_SAGE_BLACKLIST_TYPE_FROM_SERIALIZATION |
| SYS_SAGE_BLACKLIST_TEMPLATED_TYPE_FROM_SERIALIZATION |
| SYS_SAGE_BLACKLIST_TYPE_FROM_DESERIALIZATION |
| SYS_SAGE_BLACKLIST_TEMPLATED_TYPE_FROM_DESERIALIZATION |

where you specify the types in the same way as with the macros used for specialization/registration.

To check if a (templated) type is blacklisted, use

| macros for blacklisting |
| ----------------------- |
| `sys_sage::IsBlacklistedFromSerialization<T>::value` |
| `sys_sage::IsBlacklistedFromDeserialization<T>::value` |

Sometimes a type should only be blacklisted under certain conditions.
This way one can still register and serialize/deserialize the type under certain conditions and simply disable it in others while avoiding compilation errors.
For instance, _sys-sage_ has blacklisted both `std::multimap` and `std::unordered_multimap` in case the key is not an `std::string`.
You can have have a look at the following example to implement your own predicate for conditional blacklisting:

```cpp
namespace sys_sage
{
    template </* possible template arguments of your (templated) type */>
    struct IsBlacklistedFromDeserialization</* your (templated) type */> : std::bool_constant</* your predicate */> {};
}
```

### Non-typed Template Arguments

First of all, one can specialize/register every fully instantiated instance of the templated type explicitly.
Let's consider `std::array<typename T, std::size_t N>` for example. We can identify every instance needed an register everything manually:

```cpp
SYS_SAGE_REGISTER_TYPE_TRAIT(std::array<int, 3>)
SYS_SAGE_REGISTER_TYPE_TRAIT(std::array<int, 4>)
SYS_SAGE_REGISTER_TYPE_TRAIT(std::array<std::string, 4>)
...
```

In order to register a templated type in a similar fashion to `SYS_SAGE_REGISTER_TYPE_TRAIT`, where you only have to specify the templated type once and all specific instances will be handled automatically, one needs to rely on **non-portable** functionalities that depend on the compiler.
The relevant macros are

| macros for implicit/explicit non-portable type registry |
| ------------------------------------------------------- |
| SYS_SAGE_SPECIALIZE_TYPE_TRAIT_NON_PORTABLE |
| SYS_SAGE_REGISTER_TYPE_TRAIT_NON_PORTABLE |
| SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT_NON_PORTABLE |
| SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT_NON_PORTABLE |

Both `SYS_SAGE_SPECIALIZE_TYPE_TRAIT_NON_PORTABLE` and `SYS_SAGE_REGISTER_TYPE_TRAIT_NON_PORTABLE` are usually not needed, but provided nevertheless for completeness sake.
To register `std::array<typename T, std::size_t N>`, we would do

```cpp
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT_NON_PORTABLE(std::array, (typename, T), (std::size_t, N))
```

Note that we have to specify each template argument in a pair to indicate whether it is typed or non-typed.
Both `SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT_NON_PORTABLE` and `SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT_NON_PORTABLE` can support up to 128 template arguments.

Since these macros use compiler-specific information to generate the meta data, JSON that was dumped by a program compiled with one compiler **may not be deserializable** by a program compiled with another compiler.
Use these macros at your own risk.
As a general rule of thumb, if you consistently use the same compiler and the same versions, you have nothing to worry about.
If the versions don't match or if you use different compilers, it is worth checking if the different compilers generate compatible meta data.
For that, simply check whether the compilers output the same `sys_sage::TypeTrait<T>::id` for the type `T`.
Discrepencies often only occur between entirely different compilers (e.g. gcc vs clang) on types from the STL.
User-defined types should generally not pose any problems and differences between different compiler versions is very unlikely.

### Python Bindings

As of now, only a selected few types are supported for serializing/deserializing attributes, as specified by the `pybind11-json` library.
These types include

| supported types in Python |
| ------------------------- |
| None |
| bool |
| int |
| float |
| str |
| tuple |
| list |
| dict |

User-defined types are currently excluded.
We are working on a solution for this.
In the meantime, one can instead use a dictionary representing the object of a user-defined type, e.g.

```Python
import py_sys_sage as pysage

class Foo:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def PrintFoo(self):
        print(f"Foo: {self.x}, {self.y}")

comp = pysage.Component()
comp.SetAttribute("foo", Foo(1, 2).__dict__)

foo = Foo(**comp.GetAttribute("foo"))
foo.PrintFoo()
```

### Generating Meta Data for Custom Type Specializations & Registrations

TODO.

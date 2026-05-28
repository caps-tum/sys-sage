# Using attributes in sys-sage

To enable flexible extensions and customizations of components and relations,
the _sys-sage_ library allows users to insert and manipulate arbitrary
key-value attributes. These attributes can be used to make the _sys-sage_
library fit each user's unique requirements.

An overview of the API is given below.

| API |
| --- |
| sys_sage::Component::SetAttribute |
| sys_sage::Component::GetAttribute |
| sys_sage::Component::UpdateAttribute |
| sys_sage::Component::GetAttributesSize |
| sys_sage::Component::AttributesBegin |
| sys_sage::Component::AttributesEnd |
| sys_sage::Component::EraseAttribute |
| sys_sage::Component::ClearAttributes |

The `Relation` class has an analogous API.

## Inserting & Retrieving Attributes

The following code snippet showcases the simple usage of attributes:

```cpp
sys_sage::Component comp;

// insert an integer
comp.SetAttribute("foo", 10);

// insert a string
comp.SetAttribute("bar", "hello world");

// check for the integer and print it
if (int *i = comp.GetAttribute<int>("foo"))
    std::cout << *i << '\n';

// retrieve the string
const char *str = comp.GetAttribute<const char *>("bar");
```

Attributes of arbitrary type can be inserted by using the `SetAttribute` method
with a corresponding key of type `std::string`. This also includes user-defined
types. The type of the value will automatically be inferred by the compiler.
Furthermore, the `GetAttribute` method returns a pointer to the value, which
may be `nullptr` if no attribute is currently associated with the given key or
when there is a type mismatch between the requested type and the actual type of
the stored value. Note that the user must specify the exact type when using
`GetAttribute`. This is shown in the example with the string. The compiler
infers the type of the string to be `const char [12]`. Internally, we
explicitely decay the inferred type using `std::decay_t<T>`. So _sys-sage_ will
see `const char *` and only store a pointer to the string literal. One may
instead provide the type explicitely, in which case something like this would
be possible:

```cpp
comp.SetAttribute<std::string>("bar", "hello world");
```

where now a new object of type `std::string` is created which stores a copy of
the string literal. Moreover, _sys-sage_ supports move semantics to efficiently
transfer data (ownership) of attributes:

```cpp
std::vector<double> v { -0.5, 0.5 };
// move a vector into the map
comp.SetAttribute("movedVector", std::move(v));
```

Lastly, the `UpdateAttributes` method can be used to overwrite an existing
attribute with a new value. It will try to reuse memory to avoid unnecessary
memory overhead. Nevertheless, this only works if the type of the old value and
the new value match. If this is not the case, `SetAttribute` should be used
instead.

## Iterators & Attributes Size

The methods `AttributesBegin()` and `AttributesEnd()` can be used to iterate
over the attributes:

```cpp
for (auto it = comp.AttributesBegin(); it != comp.AttributesEnd(); it++) {
    std::string &key = it->first;
    if (int *value = comp.GetAttribute<int>(it))
        std::cout << key << " -> " << *value << '\n'; // print all integers
}
```

Moreover, the number of stored attributes can be retrieved by `GetAttributesSize()`.

## Removing Attributes

Individual attributes can be removed explicitly through the `EraseAttribute`
method by either provding the corresponding key or the iterator. To get rid of
all attributes at once, one can use the `ClearAttributes` method. Apart from
that, the destructor of the `Component` and `Relation` class will implicitly
clean up all remaining attributes.

However, **the user is responsible for freeing attributes that do not own their
underlying resources, such as raw pointers**. The following example illustrates
such a case in which memory will be leaked:

```cpp
int *i = new int(1);

comp.SetAttribute("leakingMemory", i);
comp.EraseAttribute("leakingMemory"); // does not free the integer itself

// manual `delete i;` needed
```

This is because the attribute's value is a shallow copy of a pointer without
taking ownership of the underlying memory. When removing this attribute, only
the memory used for storing the pointer will be freed, without further deleting
the memory of the actual integer. In such cases, further logic is needed for
correct clean-up. Apart from manually deleting `i`, **one can also use smart
pointers** in combination with `std::move` for this:

```cpp
auto i = std::make_unique<int>(1);

comp.SetAttribute("nonLeakingMemory", std::move(i)); // transfers ownership
comp.EraseAttribute("nonLeakingMemory"); // frees the integer
```

# Object Lifetime of Components and Relations

This document contains general information about the object creation and destruction of components and relations within the _sys-sage_ library.
This should be taken into account for correct memory management.

## Ownership & Correct Clean-Up

Components do not take ownership of other components and relations.
Therefore, the destructor of the `Component` class will only **unlink this component from its parent, its children and all of its associated relations**, additionally to cleaning up any claimed resources.
This also means that the construction order of components does not matter at all (e.g. a component child can be instantiated before its parent).
If the component should be cleaned up along with all of its associated relations, the `Component::Delete` functions should be used.
This function assumes that the component itself and all the relations are **heap-allocated**.
Moreover, if it is desired to delete the entire subtree spanned by a component, then the `Component::DeleteSubtree` function should be used.
This will also delete all the relations of the subcomponents. 
Similarly, this function assumes that all objects are **allocated on the heap**.

Apart from this, relations do not take ownership of components.
Hence, the destructor of the `Relation` class simply **unlinks itself from the components** in addition to cleaning up claimed resources.
It has no influence on other relations.
For consistency purposes, we also provide a `Relation::Delete` function, which is a simple wrapper around a call to `operator delete` on the given relation.
Naturally, this assumes **heap allocation**.

## Stack-allocated vs. heap-allocated

_sys-sage_ allows users to freely create components and relations on the stack or on the heap.
However, mixing stack-allocated and heap-allocated objects needs to be done with care.
An overview of functions that assume heap-allocation is given below:

- `Component::Delete`
- `Component::DeleteSubtree`
- `Component::DeleteRelations`
- `Relation::Delete`

If for instance a child component is a local stack variable while the parent is heap-allocated, then `Component::DeleteSubtree` should only be called when the child exits the scope in which it was instantiated.
The following examples highlight different scenarios of constructing and destroying components.

```cpp
#include <sys-sage.hpp>

int main()
{
    {
        // first create the parent on the stack and then the child

        sys_sage::Component parent;
        sys_sage::Component child (&parent);
    }

    {
        // first create the child on the stack and then the parent

        sys_sage::Component child;
        sys_sage::Component parent;
        parent.InsertChild(&child);
    }

    {
        // create the parent and child on the heap

        auto parent = new sys_sage::Component;
        auto child = new sys_sage::Component(parent);

        // `delete` can be called in any order
        delete parent;
        delete child;
    }

    {
        // aggregate deletion of the Component Tree

        auto child = new sys_sage::Component;
        auto parent = new sys_sage::Component;
        parent->InsertChild(child);

        sys_sage::Component::DeleteSubtree(parent);

        // `parent` and `child` are deleted
    }

    {
        // create the parent on the heap and the child on the stack

        auto parent = new sys_sage::Component;
        sys_sage::Component child (parent);

        delete parent; // does not falsly clean up the child

        // child is cleaned up at scope exit
    }

    {
        // create the parent on the heap and the child on the stack in another scope

        auto parent = new sys_sage::Component;
        {
            sys_sage::Component child (parent);

            // child gets removed from the Component Tree and is cleaned-up at exit scope
        }

        sys_sage::Component::DeleteSubtree(parent); // no problems
    }

    {
        // create the parent on the stack and the child on the heap

        sys_sage::Component parent;
        auto child = new sys_sage::Component(&parent);

        sys_sage::Component::DeleteSubtree(&parent, true); // only deletes the child without falsly trying to deallocate the parent
    }

    return 0;
}
```

## Smart pointers

Since _sys-sage_ does not enforce owenership, using smart pointers may not yield the expected outcome.
Consider the following:

```cpp
#include <sys-sage.hpp>
#include <memory>

int main()
{
    auto node = std::make_unique<sys_sage::Node>();

    sys_sage::parseHwlocOutput(node.get(), "path/to/topo.xml");

    return 0;

    // does not correctly clean up the entire Component Tree at scope exit
}
```

Since the destructor of `comp` only unlinks itself from the subtree without deleting it, the above would technically leak memory.
For this purpose, a custom deleter can be built with `Component::DeleteSubtree`.
A possible implementation would be:

```cpp
#include <sys-sage.hpp>
#include <memory>

struct Deleter
{
    void operator()(sys_sage::Component *comp) const
    {
        sys_sage::Component::DeleteSubtree(comp);
    }
};

int main()
{
    std::unique_ptr<sys_sage::Node, Deleter> node (new sys_sage::Node);

    sys_sage::parseHwlocOutput(node.get(), "path/to/topo.xml");

    return 0;

    // correctly cleans up the entire Component Tree at scope exit
}
```

# Tutorial 02: Component Tree Navigation and Querying Static Topological Information

This tutorial tries to showcase more of _sys-sage_'s API used for traversing the tree or retrieving information.
We will only focus on a selected few operations to get a feel for the Component Tree and refer to the API documentation for the full set of supported operations.

## Brief Overview of _sys-sage_'s Component API

The API for creating, manipulating, and querying Components is realized in the form of public methods of the sys_sage::Component class, hence queries are typically executed from the Component's perspective.
General functionality is available to all ComponentTypes via the common `sys_sage::Component` base class.
The API of a Component can be classified into

- creation & destruction of Components
- navigation & modification of the Component Tree
- aggregation of subtree information
- retrieval of Relations
- modification of member variables and attributes
- additional ComponentType-specific functionality

## Creation & Destruction of Components

This has already been discussed in detail in the [1st tutorial](Tutorial_01.md).

## ComponentType Retrieval

By using the sys_sage::Component::GetComponentType, one can determine the type of the Component, e.g. Storage, Cache, Core, Thread, etc.
Additionally, you can call the sys_sage::Component::GetComponentTypeStr to print the type.

## Navigation & Modification of the Component Tree

Traversal methods locate Components in the tree.
Methods like sys_sage::Component::GetParent let you traverse the tree up the levels.
Starting from a CPU core for instance, you can iterate through the cache hierarchy:

```cpp
sys_sage::Component *parent = curr->GetParent();

while (parent != nullptr) {
    if (parent->GetComponentType() == sys_sage::ComponentType::Cache) {
        sys_sage::Cache *cache = static_cast<sys_sage::Cache *>(parent);
        std::cout << cache->GetCacheName() << " cache line size: " << cache->GetCacheLineSize() << " B\n";
    } else {
        break; // reached end of cache hierarchy
    }

    curr = parent;
    parent = curr->GetParent();
}
```

Navigating down the tree can be done via the sys_sage::Component::GetChildren method.
Therefore, every target Component in the tree can be accessed from any source Component through an arbitrary chain of `sys_sage::Component::GetParent` and `sys_sage::Component::GetChildren`

Apart from this, filter operations can be used to query a set of Components that fullfil a certain condition.
For example, you can retrieve the component n levels above with the sys_sage::Component::GetNthAncestor method, or you can collect all Components of a specific type further down the levels:

```cpp
std::vector<sys_sage::Component *> qubits = node->FindDescendantsByType(sys_sage::ComponentType::Qubit);
for (sys_sage::Component *qubit : qubits) {
    sys_sage::Qubit *q = static_cast<sys_sage::Qubit *>(qubit);
    std::cout << "Qubit " << q->GetId() << "readout fidelity: " << q->GetReadoutFidelity() << '\n';
}
```

To make dynamic adjustments to the Component Tree, such as insert new Components or remove existing ones, you can make use of methods such as sys_sage::Component::InsertChild or sys_sage::Component::RemoveChild respectively.

## Aggregation

Functionality providing access to aggregate information about a collective of Components is also available through the API.

As an example, let's say we have a data intensive application and we want to split the total workload into L2-cache-resident partitions that are assigned to the underlying HW threads of the L2 cache.
For this reason, we need to determine the partition size for each L2 cache.
We can achieve this by effectively grouping the HW threads based on the shared L2 cache and count their numbers each:

```cpp
std::vector<sys_sage::Component *> caches = cpu->FindDescendantsByType(sys_sage::ComponentType::Cache);
// only keep the L2 caches
std::erase_if(caches, [](sys_sage::Component *cache){
    sys_sage::Cache *c = static_cast<sys_sage::Cache *>(cache);
    return c->GetCacheLevel() == 2;
});

std::vector<long long> partitionSizes (caches.size());

for (int i = 0; i < caches.size(); i++) {
    sys_sage::Cache *l2Cache = static_cast<sys_sage::Cache *>(caches[i]);
    
    int numHWThreads = l2Cache->CountDescendantsByType(sys_sage::ComponentType::Thread);
    partitionSizes[i] = l2Cache->GetCacheSize() / numHWThreads;
}
```

The key part here is that we identified all (CPU) L2 caches of the node and counted the number of threads "below" each L2 cache with the sys_sage::Component::CountDescendantsByType method.

## Accessing Relation

As mentioned in the documentation, Relations are usually managed through the relevant Components.
For this purpose, methods such as sys_sage::Component::GetRelationsByType or sys_sage::Component::DeleteRelations.
More advanced features, like querying all PAPI metrics through sys_sage::Component::FindPAPIrelationsInSubtree, are discussed in upcoming tutorials.

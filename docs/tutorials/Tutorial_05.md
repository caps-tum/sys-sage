# Tutorial 05: Adding 3rd Party Extensions and Processing Dynamic System Properties

This tutorial will briefly illustrate how you can further integrate 3rd party libraries into _sys-sage_ to manage dynamic system properties that are relevant to your use case.

## Mapping to the Internal Representation

At first, it is important to decide how the data provided by the 3rd party library should be stored/represented within _sys-sage_'s Internal Representation.
This can range from simply attaching a new attribute to a Component or defining new data structures and semantics.

For this example, let us consider a library that gives us information about the power zones of a node (e.g. Linux's DTPM framework).
Let's say that the CPU cores are logically partitioned into distinct power zones, each carrying information about the aggregate energy consumption and corresponding min/max limits.
We want to capture this information in _sys-sage_ by wrapping the libraries API calls around some additional logic that integrates the provided data into _sys-sage_.

Since a power zone usually spans multiple components/cores, we would like to use a Relation to model this logical relationship.
However, _sys-sage_ does not provide a RelationType out-of-the-box that is tailored for this specific use case, as we need to store power-related metrics.
Luckily, we can take a "plain" Relation and customize it by defining a new RelationCategory and by making use of a Relation's attribute to store custom data.
This way we model a power zone via a Relation that works as an interface providing all the necessary information.
We define our new category to be

```cpp
namespace sys_sage::RelationCategory
{
    constexpr type PowerZone = 10;
}
```

where we have made sure that the value 10 is not reserved by any other category.

## Integrating the 3rd Party Library

For the sake of this (simplified) tutorial, let's assume that our hypothetical 3rd party library has two functions: `int GetPowerZonePartitioning(int ***partitionnig)` and `int MeasurePowerZoneMetrics(int powerZoneId, struct PowerZoneMetrics *measurement)`.
The former one gives us the logical partitioning of the cores via a 2D array, where the first dimension denotes the ID of the power zone in incremental order and the second one contains the cores that are part of that power zone.
Also, by convention the end of the "power zone array" is `NULL` and the end of the "core arrays" is denoted by -1, e.g.

```
  power zone ID                     core IDs
                  ------      --------------------
              0  |      | -> | 0 | 1 | 2 | 3 | -1 |
                 |------|     --------------------
              1  |      | -> | 4 | 5 | 6 | 7 | -1 |
                 |------|     --------------------
                 | NULL |
                  ------
```

The 2D array is allocated by the library and the pointer argument is set to point to it.
The latter function fills a given struct with some currently measured data, containing current energy counter in µJ and the min/max power limits, for a given power zone.
If an error occurs both function return 1, otherwise 0.

We decide to encapsulate `GetPowerZonePartitioning` inside of a wrapper that returns the suitable _sys-sage_ abstractions, e.g. we map the 2D array to a vector of Relations:

```cpp
int sys_sage_GetPowerZonePartitioning(sys_sage::Component *cpu, std::vector<sys_sage::Relation> &powerZones)
{
    powerZones.clear(); // make sure the vector is empty

    int **twoDArray = nullptr;
    int rval = GetPowerZonePartitioning(&twoDArray);
    if (rval != 0)
        return rval; // simply propagate the error code

    for (int i = 0; twoDArray[i] != nullptr; i++) { // iterate over power zones
        int powerZoneId = i;
        std::vector<sys_sage::Component *> cores;

        for (int j = 0; twoDArray[i][j] != -1; j++) { // iterate over cores
            int coreId = twoDArray[i][j];
            sys_sage::Component *core = cpu->GetDescendantById(coreId, sys_sage::ComponentType::Core); // find the core with the given ID in the topology
            if (core == nullptr) {
                std::cerr << "error: could not find CPU core with ID " << coreId << '\n';
                return -1; // return error code that is different from the error codes used by the 3rd party library to express sys-sage integration error
            }

            cores.push_back(core);
        }

        int relationId = -1; // we don't care about the ID of the relation
        bool ordered = false; // the order of the cores in the relation does not matter
        sys_sage::RelationCategory::type category = sys_sage::RelationCategory::PowerZone; // our user-defined category
        powerZones.emplace_back(cores, relationId, ordered, category); // create new relation

        sys_sage::Relation &powerZone = powerZones.back(); // access the newly created relation
        powerZone.SetAttribute("powerZoneId", powerZoneId); // store the power zone ID as an attribute
    }

    freeTwoDArray(twoDArray);
    return 0; // return the same success value that is used by the 3rd party library
}
```

For `MeasurePowerZoneMetrics`, we want to store the metrics as an attribute of the Relation.
Let's assume that we don't care about the previous measurement anymore when calling this function and can simply overwrite it.
Moreover, since a Relation now represents a power zone, we can make our wrapper take a Relation as the argument:

```cpp
int sys_sage_MeasurePowerZoneMetrics(sys_sage::Relation &powerZone)
{
    if (powerZone.GetCategory() != sys_sage::RelationCategory::PowerZone) // only consider relations that model a power zone
        return -1;

    int *powerZoneId = powerZone.GetAttribute<int>("powerZoneId");
    if (powerZoneId == nullptr)
        return -2;

    PowerZoneMetrics *measurement = powerZone.GetAttribute<PowerZoneMetrics>("measurement");
    if (measurement == nullptr) // no measurement yet
        measurement = powerZone.SetAttribute<PowerZoneMetrics>("measurement", PowerZoneMetrics{}); // insert an empty struct

    int rval = MeasurePowerZoneMetrics(*powerZoneId, measurement);
    if (rval != 0)
        return rval; // simply propagate the error code

    return 0; // return the same success value that is used by the 3rd party library
}
```

## Using the Integration

Now you can simply do the following:

```cpp
std::vector<sys_sage::Relation> powerZones;
sys_sage_GetPowerZonePartitioning(cpu, powerZones);

for (sys_sage::Relation &powerZone : powerZones) {
    sys_sage_MeasurePowerZoneMetrics(powerZone);
    int *powerZoneId = powerZone.GetAttribute<int>("powerZoneId");
    PowerZoneMetrics *measurement = powerZone.GetAttribute<PowerZoneMetrics>("measurement");

    std::cout << "aggregate energy counter on power zone " << *powerZoneId << ": " << measurement->energy_uj << " µJ\n";
}
```

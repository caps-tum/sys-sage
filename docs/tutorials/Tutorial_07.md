# Tutorial 07: Basics of _sys-sage_'s PAPI extension

We will turn our attention towards the PAPI extension of the _sys-sage_ library and how to use it at a fundamental level.

Let us consider this simple piece of code:

```cpp
#include <memory>
#include <stdlib.h>

static constexpr int ITER = 3;

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
    for (size_t i = 0; i < n; i++)
        a[i] = alpha * b[i] + c[i];
}

int main(int argc, const char **argv)
{
    size_t n = 1'000'000;
    auto a = std::make_unique<double[]>(n);
    auto b = std::make_unique<double[]>(n);
    auto c = std::make_unique<double[]>(n);
    double alpha = 3.14159;

    for (int i = 0; i < ITER; i++) {
        saxpy(a.get(), b.get(), c.get(), n, alpha);
    }

    return EXIT_SUCCESS;
}
```

We are interested in profiling the performance of the code at the loop in which we repeatedly perform some computation.
Apart from simply capturing the performance counters, we actually want to trace where the counters have been monitored on, i.e. which hardware threads were involved.
For this reason, we will make use of the combined _sys-sage_-PAPI integration.

As mentioned in the [documentation](../external_interfaces/PAPI.md), the PAPI extension of _sys-sage_ is covered by two APIs: standalone ("plain") PAPI and the _sys-sage_-PAPI API.
Let us cover both APIs one-by-one.

## Plain PAPI

As with any other application that is profiled with PAPI, we need to initialize the library, set up the event set and at the end free the resources and terminate the library.
This is all handled through plain PAPI, i.e. without any wrappers or extensions from _sys-sage_'s side.
The modified code is as follows:

```cpp
#include <papi.h>
#include <iostream>
#include <memory>
#include <stdlib.h>

static constexpr int ITER = 3;

#define FATAL(errMsg) do {\
    std::cerr << "error: " << (errMsg) << '\n';\
    return EXIT_FAILURE;\
} while (false)

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
    for (size_t i = 0; i < n; i++)
        a[i] = alpha * b[i] + c[i];
}

int main(int argc, const char **argv)
{
    size_t n = 1'000'000;
    auto a = std::make_unique<double[]>(n);
    auto b = std::make_unique<double[]>(n);
    auto c = std::make_unique<double[]>(n);
    double alpha = 3.14159;

    int rval = PAPI_library_init(PAPI_VER_CURRENT);
    if (rval != PAPI_VER_CURRENT)
        FATAL(PAPI_strerror(rval));

    int eventSet = PAPI_NULL;
    rval = PAPI_create_eventset(&eventSet);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    int events[] = {
        PAPI_TOT_INS,
        PAPI_TOT_CYC
    };
    int numEvents = sizeof(events) / sizeof(events[0]);
    rval = PAPI_add_events(eventSet, events, numEvents);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    for (int i = 0; i < ITER; i++) {
        saxpy(a.get(), b.get(), c.get(), n, alpha);
    }

    rval = PAPI_cleanup_eventset(eventSet);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));
 
    rval = PAPI_destroy_eventset(&eventSet);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    PAPI_shutdown();

    return EXIT_SUCCESS;
}
```

This is all basic PAPI up to this point.
Please refer to the PAPI documentation if the above is not familiar to you, since this tutorial will focus on the _sys-sage_ integration.

## _sys-sage_-PAPI API

Let's provide the full implementation first and then go over the additions made to the code.
The added _sys-sage_-PAPI API calls are surrounded by subsequent `/` symbols for clarity.

```cpp
#include <sys-sage.hpp>
#include <iostream>
#include <memory>
#include <stdlib.h>

static constexpr int ITER = 3;

#define FATAL(errMsg) do {\
    std::cerr << "error: " << (errMsg) << '\n';\
    return EXIT_FAILURE;\
} while (false)

void saxpy(double *a, const double *b, const double *c, size_t n, double alpha)
{
    for (size_t i = 0; i < n; i++)
        a[i] = alpha * b[i] + c[i];
}

int main(int argc, const char **argv)
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <path_to_hwloc_xml>\n";
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////////////
    sys_sage::Node *node = new sys_sage::Node;
    if (sys_sage::parseHwlocOutput(node, argv[1]) != 0)
        return EXIT_FAILURE;
    ////////////////////////////////////////////////////

    size_t n = 1'000'000;
    auto a = std::make_unique<double[]>(n);
    auto b = std::make_unique<double[]>(n);
    auto c = std::make_unique<double[]>(n);
    double alpha = 3.14159;

    int rval = PAPI_library_init(PAPI_VER_CURRENT);
    if (rval != PAPI_VER_CURRENT)
        FATAL(PAPI_strerror(rval));

    int eventSet = PAPI_NULL;
    rval = PAPI_create_eventset(&eventSet);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    int events[] = {
        PAPI_TOT_INS,
        PAPI_TOT_CYC
    };
    int numEvents = sizeof(events) / sizeof(events[0]);
    rval = PAPI_add_events(eventSet, events, numEvents);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    ///////////////////////////////////////////////////
    sys_sage::Relation *metrics = nullptr;
    rval = sys_sage::SS_PAPI_start(eventSet, &metrics);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));
    ///////////////////////////////////////////////////

    for (int i = 0; i < ITER; i++) {
        saxpy(a.get(), b.get(), c.get(), n, alpha);

        ////////////////////////////////////////////////////
        rval = sys_sage::SS_PAPI_read(metrics, node, true);
        if (rval != PAPI_OK)
            FATAL(PAPI_strerror(rval));
        ////////////////////////////////////////////////////
    }

    // stop the event set without storing perf counters -> use plain PAPI_stop
    rval = PAPI_stop(eventSet, nullptr);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    rval = PAPI_cleanup_eventset(eventSet);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));
 
    rval = PAPI_destroy_eventset(&eventSet);
    if (rval != PAPI_OK)
        FATAL(PAPI_strerror(rval));

    ////////////////////////////
    metrics->PrintPAPImetrics();
    ////////////////////////////

    PAPI_shutdown();

    sys_sage::Component::DeleteSubtree(node);

    return EXIT_SUCCESS;
}
```

As usual, we first create the _sys-sage_ topology model by parsing the XML file provided by hwloc.

Now, in order to attribute the performance counter values to the hardware components, the _sys-sage_ library uses a Relation to naturally link them together.
For this purpose, a pointer of type sys_sage::Relation is provided to sys_sage::SS_PAPI_start.
Since `metrics` is `nullptr`, the _sys-sage_ library will create a new relation object, which `metrics` will point to after the call to `sys_sage::SS_PAPI_start`.
If it already points to a valid Relation object, that object will be reused instead.
Within the same call, the underlying `PAPI_start` routine will be used to start the event set.
This will also bind the Relation object to the given event set.
Note that when bound to new event set, the Relation object does not loose the metrics of the previous event set.

Within the loop, we repeatedly measure the current counters of the relevant events by using the sys_sage::SS_PAPI_read function.
Since the event set is now represented by the Relation, we forward the Relation object to the wrapper to integrate and correlate the counter values and corresponding hardware threads.
A pointer to the `node` object, which represents the root of our topology, is also passed to the wrapper to search for the hardware thread abstractions inside of the topology model.
Lastly, the boolean parameter is set to `true`, since we want to take multiple snapshots of the performance counters without overwriting the previous ones.

We also point out that we wanted to stop the profiling without storing any new metrics, since the counters after the loop are irrelevant to us.
Therefore, we use plain `PAPI_stop`, with `nullptr` as one of the arguments, instead of sys_sage::SS_PAPI_stop.

Information about the performance counter values and the associated hardware threads can be obtained through the Relation object or through the Component Tree.
Through the integration of PAPI into _sys-sage_, the obtained metrics can be analyzed within a coherent model of the hardware architecture.
In this case, this information is simply printed using `sys_sage::Relation::PrintPAPImetrics`.

Note that the PAPI related functions offered in the _sys-sage_ API will not work if the PAPI library is shut down.

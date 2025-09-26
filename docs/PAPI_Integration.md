# PAPI Integration

The _sys-sage_ library provides routines corresponding to the known `PAPI_read`,
`PAPI_accum`, `PAPI_stop` functions to capture the hardware performance
counters on CPUs. Their function signatures contain:

```cpp
int sys_sage::PAPI_read(int eventSet, Component *root,
                        Thread **outThread = nullptr,
                        unsigned long long *outTimestamp = nullptr);

int sys_sage::PAPI_accum(int eventSet, Component *root,
                        Thread **outThread = nullptr,
                        unsigned long long *outTimestamp = nullptr);

int sys_sage::PAPI_stop(int eventSet, Component *root,
                        Thread **outThread = nullptr,
                        unsigned long long *outTimestamp = nullptr);
```

where `root` is a pointer to the root of the hardware topology and `outThread`
as well as `outTimestamp` are optional output-parameters whose purpose will be
explained later.

The above functions offer a way to automatically integrate the extracted
performance counters into the _sys-sage_ topology without extra effort on the
user's side, thus coupling the metrics directly to the relevant hardware
components and putting them into the context of the overall hardware topology.
They can be thought of as wrapper functions around the actual PAPI routines.

## General Workflow

It's up to the user to initialize the PAPI library and to create and configure
the event sets through the plain PAPI. Additionally, the user may use the known
`PAPI_start` routine to start performance monitoring. These functions have
intenionally not been adopted by _sys-sage_, as they do not read the
performance counters.

A simple example would be:

```cpp
#include "sys-sage.hpp"
#include <papi.h>
#include <iostream>
#include <memory>
#include <stdlib.h>

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

  sys_sage::Node node;
  if (sys_sage::parseHwlocOutput(&node, argv[1]) != 0)
    return EXIT_FAILURE;

  int rval;

  size_t n = 1'000'000;
  auto a = std::make_unique<double[]>(n);
  auto b = std::make_unique<double[]>(n);
  auto c = std::make_unique<double[]>(n);
  double alpha = 3.14159;

  rval = PAPI_library_init(PAPI_VER_CURRENT);
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

  sys_sage::Thread *thread;

  rval = PAPI_start(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  saxpy(a.get(), b.get(), c.get(), n, alpha);

  rval = sys_sage::PAPI_stop(eventSet, &node, &thread);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  thread->PrintPAPICounters();

  rval = PAPI_cleanup_eventset(eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  rval = PAPI_destroy_eventset(&eventSet);
  if (rval != PAPI_OK)
    FATAL(PAPI_strerror(rval));

  return EXIT_SUCCESS;
}
```

### Under the Hood

The routines `sys_sage::PAPI_read`, `sys_sage::PAPI_accum`,
`sys_sage::PAPI_stop` all follow a very similar strategy:

1. Based on the given event set, determine the events associated to it and
   store the event codes in a local array called `events`.

2. Perform the call to the underlying PAPI routine.

   - `sys_sage::PAPI_read`  -> `PAPI_read`

   - `sys_sage::PAPI_accum` -> `PAPI_accum`

   - `sys_sage::PAPI_stop`  -> `PAPI_stop`

   For this, the counters are first written into a local array called `counters`.
   Note that in the case of `PAPI_accum`, we need to provide an array with
   0-valued entries, since the accumulation needs not to be done on temporary
   data, but on the previously stored counter values, which are queried from
   the _sys-sage_ topology at a later point.

3. Depending on the event set, figure out which hardware thread the counters
   belong to and find its ID. Here we need to make a case destinction:

   - If the event set has explicitely been attached to a hardware thread,
     simply query for the ID with PAPI.

   - If the event set has explicitely been attached to a software thread, get
     the last known hardware thread on which it was scheduled on by reading
     `/proc/<tid>/stat`.

   - Otherwise, the event set is implicitely attached to the current software
     thread, in which case we simply call `sched_getcpu()`.

   Except for the first case, it could happen that the event set monitors
   performance counters on different hardware threads through repeated
   re-scheduling of the corresponding software thread. This leads into the
   fact that the performance counter values of different readings may be
   scattered onto different hardware threads.

4. Together with the ID of the hardware thread, query for its handle in the
   _sys-sage_ topology. This handle is recorded into `outThread` for the user,
   if it is not `nullptr`.

5. Store the values of `counters` into the `attrib` map of the hardware thread
   on a per-event basis, meaning that if the value `counters[i]` at index `i`
   corresponds to the event `events[i]`, we will have a key-value pair of
   `{ events[i], counters[i] }`. Note that the string representation of the
   event code is used as the actual key. Further details about the storage
   mechanism is given later.

### Multiple Counter Readings

Per default, an event will be linked to only a single performance counter value
at a time. This approach offers simplicity and reduces memory usage.

Alternitavely, _sys-sage_ allows the user to store multiple performance counter
readings of the same event. This option can be set through (TODO: finish
sentence). To distinguish them from one another, timestamps have been used
which are recorded into `outTimestamp`, if it is not `nullptr`. The user can
utilize the timestamp to retrieve a single counter value associated to the
corresponding reading. It is important to note that these timestamps are *not*
guaranteed to be unique -- although most likely they will -- and in case of a
collision, the one who was stored first will be returned (TODO: maybe return
the last/most recent one?). On the other hand, all readings can be provided to
the user at once.

### Storing the counters

For every event in the event set, _sys-sage_ follows the below procedure:

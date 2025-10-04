# PAPI Integration

The _sys-sage_ library provides routines corresponding to the known `PAPI_read`,
`PAPI_accum` and `PAPI_stop` functions to capture hardware performance counters
on CPUs. Their function signatures contain:

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
user's side, thus attributing the metrics directly to the relevant hardware
components and putting them into the context of the overall hardware topology.
They can be thought of as wrapper functions around the actual PAPI routines.

## General Workflow

The following diagram shows the overall workflow of the PAPI metrics collection
and evaluation through _sys-sage_:

![Workflow of PAPI under sys-sage](./images/sys-sage_PAPI_workflow.pdf)

The green boxes correspond to the _sys-sage_ API whereas the blue ones
correspond to plain PAPI. An example of the basic usage can be found in the
`<path_to_sys-sage>/examples/papi_basics.cpp` file.

## Under the Hood

The routines `sys_sage::PAPI_read`, `sys_sage::PAPI_accum` and
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
   performance counters on different hardware threads caused by repeated
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

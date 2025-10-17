# PAPI Integration

The _sys-sage_ library provides routines corresponding to the known `PAPI_read`,
`PAPI_accum` and `PAPI_stop` functions to capture hardware performance counters
on CPUs. The function signatures include

```cpp
int sys_sage::PAPI_read(int eventSet, unsigned long long *timestamp,
                        Component *root, PAPIMetrics **metrics);

int sys_sage::PAPI_accum(int eventSet, unsigned long long *timestamp,
                        Component *root, PAPIMetrics **metrics);

int sys_sage::PAPI_stop(int eventSet, unsigned long long *timestamp,
                        Component *root, PAPIMetrics **metrics);
```

where `root` is a pointer to the root of the _sys-sage_ topology. The purpose
of `timestamp` and `metrics` will be explained later on.

The above functions offer a way to automatically integrate the performance
counter values into the _sys-sage_ topology, thus attributing the metrics
directly to the relevant hardware components and putting them into the context
of the overall hardware topology. They can be thought of as wrapper functions
around the actual PAPI routines.

## General Workflow

The following diagram shows the overall workflow of the PAPI metrics collection
and evaluation through _sys-sage_:

![](images/sys-sage_PAPI_workflow.png)

The green boxes correspond to the _sys-sage_ API whereas the blue ones
correspond to plain PAPI. In general, the creation and configuration of event
sets remain with the original PAPI, while the performance monitoring is now
managed through _sys-sage_. An example of the basic usage can be found in the
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

   For this, the counters are written into a local array called `counters`.
   Note that in the case of `sys_sage::PAPI_accum`, we need to provide an array
   with 0-valued entries, since the accumulation needs not to be done on
   temporary data, but on the previously stored counter values, which are
   queried from the _sys-sage_ topology at a later point.

3. Depending on the event set, figure out to which hardware thread the counters
   belong to and find its ID. Here, we need to make a case destinction:

   - If the event set has explicitely been attached to a hardware thread,
     simply query for the ID through PAPI.

   - If the event set has explicitely been attached to a software thread, get
     the last known hardware thread on which it was scheduled on by reading
     `/proc/<tid>/stat`.

   - Otherwise, the event set is implicitely attached to the current software
     thread, in which case we simply call `sched_getcpu()`.

   In the last two cases, the software thread can potentially migrate across
   multiple hardware threads through repeated re-scheduling. From PAPI's
   perspective, this doesn't pose any concern. The performance counter values
   are simply fetched from the PMU of the current hardware thread and may
   potentially be processed with the values stemming from other hardware
   threads. This results in "mixed" performance counter values. If this
   behaviour is not desired, the user should consider thread affinity and
   pinning for more homogeneous performance monitoring.

   To put possibly "mixed" performance counter values into the context of the
   hardware threads, _sys-sage_ uses a new relation type called `PAPIMetrics`,
   which naturally "keeps track" of the hardware threads.

4. Together with the ID of the hardware thread, query for its handle in the
   _sys-sage_ topology. Furthermore, if `*metrics == nullptr`, a new relation
   of type `RelationType::PAPIMetrics` is created, which contains the handle
   and is recorded into `*metrics` for later reference by the user. Otherwise,
   _sys-sage_ inserts the handle into `*metrics` if it's not already contained.

5. Store the values of `counters` into the `attrib` map of `*metrics` on a
   per-event basis, meaning that if the value `counters[i]` at index `i`
   corresponds to the event `events[i]`, we will have a key-value pair similar
   to `{ events[i], counters[i] }`. Note that the values are actually wrapped
   around a datastructure and that the string representation of the event code
   is used as the actual key. Further details about the storage mechanism is
   given below.

### Multiple Performance Counter Readings

We define a "performance counter reading" to be the act of fetching the current
values of the performance counters. It may be triggered by a call to either
`sys_sage::PAPI_read`, `sys_sage::PAPI_accum` or `sys_sage::PAPI_stop`.

Now, the _sys-sage_ library allows the user to store the results of multiple
performance counter readings of the same event. To distinguish them from one
another, timestamps have been introduced which are recorded into `timestamp`.
A timestamp is always associated to the entire reading, meaning that
performance counter values of different events share the same timestamp within
the same reading. Furthermore, a timestamp can be used to get the value of a
specific reading. It is important to state that these timestamps are **not**
guaranteed to be unique -- although most likely they will -- and in case of a
collision, the value of the later reading will be returned. Apart from that,
the user may also access the entire datastructure containing the values of all
readings.

Normally, a performance counter reading would not create a new entry in the
datastructure, but rather update an already existing one to reflect the latest
reading. This would either involve overwriting the previous value with a new
one or adding new values to it. Either way, the previous timestamp will always
be overwritten. Of course, if no previous entry exists, a new one will be
created automatically. On the other side, always updating an existing entry may
not be desirable, to which _sys-sage_ allows the user to indicate when a new
entry should be created. Simply put, whenever `*timestamp == 0`, a new entry is
added to the datastructure. This allows for the following:

```cpp
unsigned long long timestamp = 0;
for (int i = 0; i < ITER; i++) {
    compute();
    sys_sage::PAPI_accum(eventSet, &timestamp, &root, &metrics);
}
```

where `timestamp` now refers to the latest accumulated value of a newly created
entry.

## PAPIMetrics - A new Relation

As discussed before, `PAPIMetrics` offers a way to capture "mixed" performance
counters values while putting them into the context of the hardware threads
from which they stem from. Moreover, this new relation offers ways to access
those values. These are

```cpp
long long sys_sage::PAPIMetrics::GetPerfCounterReading(int event,
                                                       unsigned long long timestamp = 0);

std::vector<std::pair<unsigned long long, long long>> *
sys_sage::PAPIMetrics::GetPerfCounterReadings(int event);

void sys_sage::PAPIMetrics::PrintLatestPerfCounterReadings();
```

Note that `timestamp == 0` implies that the user wants the value of the latest
reading.

## Error Handling

For consistency, the error codes of PAPI have been adopted.

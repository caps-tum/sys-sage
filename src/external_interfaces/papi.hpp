#ifdef PAPI_METRICS

namespace sys_sage {
  int PAPI_read(int eventSet, Component *root, Thread **outThread = nullptr,
                unsigned long long *outTimestamp = nullptr);

  int PAPI_accum(int eventSet, Component *root, Thread **outThread = nullptr,
                unsigned long long *outTimestamp = nullptr);

  int PAPI_stop(int eventSet, Component *root, Thread **outThread = nullptr,
                unsigned long long *outTimestamp = nullptr);

  int PAPI_store(int eventSet, const long long *counters, int numCounters,
                 Component *root, Thread **outThread = nullptr,
                 unsigned long long *outTimestamp = nullptr);
}

#endif

#include <sys-sage.hpp>

#ifdef PROC_CPUINFO

#include <boost/ut.hpp>

using namespace boost::ut;
using namespace sys_sage;


static suite<"cpuinfo"> _ = []
{
    Core core;
    Thread thread{&core};

    core.SetFreq(42.0);
    expect(that % 42.0 == core.GetFreq());
    expect(that % 42.0 == thread.GetFreq());
};

#endif

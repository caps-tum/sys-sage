
#include <iostream>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <numaif.h>
#include <numa.h>
#include <sched.h>
#include <thread>
#include <sys/wait.h>
#include <bitset>

using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////
//PARAMS TO SET
#define REPEATS 5
#define LATENCY_REPEATS 256
#define MAIN_MEM_FRACTION 8//8             //allocate 1/x of main memory
#define CACHE_LINE_SZ 64
#define TIMER_WARMUP 32
#define TIMER_REPEATS 128

//#define MEASURE_EACH_CPU
////////////////////////////////////////////////////////////////////////

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE);} while (0)

struct numa_region
{
    long long numa_mem_sz;
    long long arrsz;
    int step;
    int num_elems;

    uint64_t timer_overhead;

    uint64_t latency_time;
    uint64_t bw_time;
} ;

void fill_arr(uint64_t * arr, int num_elems)
{
    for(int i=0; i<num_elems; i++)
    {
        arr[i] = i;
    }
}

uint64_t get_timer_overhead(int repeats, int warmup)
{
    high_resolution_clock::time_point t_start, t_end;
    uint64_t time = 0;
    //uint64_t time_arr[10];
    for(int i=0; i<repeats+warmup; i++)
    {
        t_start = high_resolution_clock::now();
        t_end = high_resolution_clock::now();
        if(i>=warmup)
            time += t_end.time_since_epoch().count()-t_start.time_since_epoch().count();
        // if(i<10)
        //     time_arr[i]=t_end.time_since_epoch().count()-t_start.time_since_epoch().count();

    }
    // for(int i=0; i<10; i++)
    // {
    //     cout << time_arr[i] << "; ";
    // } //cout << endl;

    time = time/repeats;

    return time;
}


int run_measurement(unsigned int src_cpu, unsigned int src_numa, int numa_nodes)
{
    uint64_t *arr, *mock_arr;
    high_resolution_clock::time_point t_start, t_end;
    struct numa_region *numa = new numa_region[numa_nodes];

    for(int n=0; n<numa_nodes; n++)
    {
        numa[n].numa_mem_sz = numa_node_size(n, NULL);
        //cout << "node " << n << "; numa_mem_sz: " << numa[n].numa_mem_sz << "; arrszsz: " << numa[n].arrsz << "; step: "<< numa[n].step << "; mmfww: " << numa[n].numa_mem_sz/MAIN_MEM_FRACTION << "; mmf: " << MAIN_MEM_FRACTION <<endl;
        numa[n].arrsz = numa[n].numa_mem_sz/MAIN_MEM_FRACTION;   //fraction of main memory to use (*2 arrays)
        numa[n].step = CACHE_LINE_SZ/sizeof(uint64_t);      //one elem per cache line
        numa[n].num_elems = numa[n].arrsz/sizeof(uint64_t);
        numa[n].timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);
        numa[n].latency_time = 0;
        numa[n].bw_time = 0;
    }

    for(int round = 0; round<REPEATS; round++)
    {
        for(int n=0; n<numa_nodes; n++)
        {
if(round == 0) cerr << "    numa" << n << ": mem_sz[mb] "<< numa[n].numa_mem_sz/(1<<20) << ", elems[k] " << numa[n].num_elems/(1<<10) << endl;
            if(numa[n].numa_mem_sz == 0)
                continue;
            int sum;
            mock_arr = (uint64_t*)numa_alloc_onnode(numa[n].arrsz, n);
            arr = (uint64_t*)numa_alloc_onnode(numa[n].arrsz, n);

            fill_arr(arr, numa[n].num_elems);
            fill_arr(mock_arr, numa[n].num_elems);

            srand (time(NULL));//initialize rand generator

            for(int i = 0; i<LATENCY_REPEATS; i++)
            {
                int rand_idx= rand()%numa[n].num_elems;
                numa[n].timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);
                t_start = high_resolution_clock::now();
                sum = mock_arr[rand_idx];
                t_end = high_resolution_clock::now();
                numa[n].latency_time += t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-numa[n].timer_overhead;
            }
            fill_arr(mock_arr, numa[n].num_elems);
            t_start = high_resolution_clock::now();
            for(int i = 0; i<numa[n].num_elems; i+=numa[n].step)
            {
                // if (i==0 || i==num_elems/8 || i==num_elems/2 || i==num_elems*7/8|| i==num_elems-1)
                // {
                //     //t_start = high_resolution_clock::now();
                //     void* ptr_to_check = (void*)&(arr[i]);
                //     int status = -1;
                //     int ret_code;
                //     ret_code=move_pages(0 /*self memory */, 1, &ptr_to_check, NULL, &status, 0);
                //     //t_end = high_resolution_clock::now();
                //     //cout << /*arrsz << " ; " <<*/ t_end.time_since_epoch().count()-t_start.time_since_epoch().count() << "; ";//endl;
                //     //printf("    %d       Memory at %p is at %d node (retcode %d)\n", status, ptr_to_check, status, ret_code);
                // }
                sum+=arr[i];
            }
            t_end = high_resolution_clock::now();
            numa[n].bw_time += t_end.time_since_epoch().count()-t_start.time_since_epoch().count()-numa[n].timer_overhead;

            numa_free(mock_arr, numa[n].arrsz);
            numa_free(arr, numa[n].arrsz);
        }
        //cerr << round+1 << "/" << REPEATS << " done" << endl;
    } //cerr << endl << endl;

    for(int n=0; n<numa_nodes; n++)
    {
        if(numa[n].numa_mem_sz == 0)
            continue;
        #ifdef MEASURE_EACH_CPU
            cout << src_cpu;
        #else
            cout << src_numa;
        #endif
        cout << "; " << n << "; " << numa[n].numa_mem_sz << "; " << numa[n].arrsz << "; " << numa[n].timer_overhead << "; " << numa[n].latency_time/(REPEATS*LATENCY_REPEATS) << "; " << numa[n].arrsz/((numa[n].bw_time/REPEATS)/1000) << endl;
    }
    delete[] numa;
    return 0;
}

int main(int argc, char *argv[])
{
    cerr << "usage: ./caps-numa-benchmark > output_file.csv     (otherwise, stdout gets mixed with stderr debug ouput)" << endl;
    cerr << "  params: MEASURE_EACH_CPU, REPEATS, LATENCY_REPEATS, MAIN_MEM_FRACTION, CACHE_LINE_SZ, TIMER_WARMUP, TIMER_REPEATS can be set only in the source code (will be improved)" << endl;

    cpu_set_t set;
    unsigned int this_cpu, this_numa;

    if(numa_available() == -1)
        return 1;

    int measure_numa_only = 1;
    #ifdef MEASURE_EACH_CPU
        measure_numa_only = 0;
    #endif

    int numa_nodes = numa_num_configured_nodes();
    const auto cpu_count = std::thread::hardware_concurrency();
    cerr << "# hw threads: " << cpu_count << ", numa nodes: " << numa_nodes << endl;
    cerr << "################################" << endl;
    #ifdef MEASURE_EACH_CPU
        cout << "src_cpu;";
    #else
        cout << "src_numa;";
    #endif
    cout << "target_numa;mem_size;arrsz;timer_ovh;ldlat(ns);bw(MB/s);" << endl;

    unsigned long long mask_checked_component = 0; //each bit is one numa region/one HW thread
    for(unsigned int current_cpu = 0; current_cpu < cpu_count; current_cpu ++)
    {
        CPU_ZERO(&set);
        CPU_SET(current_cpu, &set);
        if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
            errExit("sched_setaffinity");
        getcpu(&this_cpu, &this_numa);
        if(this_cpu != current_cpu)
            errExit("this_cpu != current_cpu");

        #ifdef MEASURE_EACH_CPU
            int mask_bit = this_cpu;
        #else
            int mask_bit = this_numa;
        #endif

        cerr << "    cpu " << this_cpu << " - mask-checked " << std::bitset<24>(mask_checked_component) << "; bit " << mask_bit << endl;
        if((mask_checked_component & (1 << mask_bit)) == 0)
        {
            mask_checked_component += (1 << mask_bit); //je jen v child process
            run_measurement(this_cpu, this_numa, numa_nodes);
        }
    }

    return 0;
}

#include <sys-sage.hpp>
#include <hwloc.h>

///for matmul
#include<iostream>
#include<iomanip>
#include<cmath>
#include <chrono>

//getcpu
#include <sched.h>

//pid
#include <unistd.h>
#include <sys/types.h>

//intel cat
#ifdef INTEL_PQOS
#include <pqos.h>
#endif

////////////////////////////////////////////////////////////////////////
//PARAMS TO SET
#define TIMER_WARMUP 32
#define TIMER_REPEATS 128

////////////////////////////////////////////////////////////////////////

using namespace sys_sage;

using namespace std;
using namespace std::chrono;

int hwloc_dump_xml(const char *filename);
uint64_t get_timer_overhead(int repeats, int warmup);

double * A;
double * B;
double * C;

void fill_matrices(const int m_dim, const int n_dim, const int l_dim)
{
    for(int x = 0; x< m_dim; x++)
        for(int y = 0; y< n_dim; y++)
            A[x + m_dim*y] = (x+1)*(y+1);

    for(int x = 0; x< l_dim; x++)
        for(int y = 0; y< m_dim; y++)
            B[x + l_dim*y] = (x+1)*(y+1);

    for(int x = 0; x< l_dim; x++)
        for(int y = 0; y< n_dim; y++)
            C[x + l_dim*y] = 0;
}

void print_matrix(double * mat, int x_dim, int y_dim)
{
    for(int y = 0; y< y_dim; y++)
    {
        for(int x = 0; x< x_dim; x++)
        {
            cout << setw(7) << mat[x + x_dim*y];
        }
        cout << endl;
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
    if(argc < 4 ){
        cerr << "usage: ./matmul m_dim n_dim l_dim [tile_sz_bytes]" << endl;
        return 1;
    }
    const int m_dim = atoi(argv[1]);
    const int n_dim = atoi(argv[2]);
    const int l_dim = atoi(argv[3]);

    high_resolution_clock::time_point t_start, t_end;
    uint64_t timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);

    int tile_sz_bytes;
    if(argc == 5)
        tile_sz_bytes = atoi(argv[4]) * 0.9 /3 ;
    else
    {
        Node* n = new Node(1);
        hwloc_dump_xml("tmp_hwloc.xml");
        if(parseHwlocOutput(n, "tmp_hwloc.xml") != 0){
            cerr << "failed parsing hwloc output" << endl; return 1;
        }

        unsigned int myCpu, myNuma;
        if(getcpu(&myCpu, &myNuma) != 0){
            cerr << "getcpu failed" << endl; return 1;
        }
        Thread * t = (Thread*)n->FindSubcomponentById(myCpu, sys_sage::ComponentType::Thread);//find current hw thread in sys-sage
        if(t==NULL){
            cerr << "HW thread " << myCpu << "not found in sys-sage" << endl; return 1;
        }

        //////////////////////////////////// whole L3 size
#ifndef INTEL_PQOS

        Component * c = (Component*)t;
        while(c->GetParent() != NULL){
            //go up until L3 found
            c = c->GetParent();
            if(c->GetComponentType() == sys_sage::ComponentType::Cache && ((Cache*)c)->GetCacheLevel() == 3)
                break;
        };
        if(c==NULL || c->GetComponentType() != sys_sage::ComponentType::Cache){
            cerr << "L3 cache not found" << endl; return 1;
        }
        long long available_L3_size = ((Cache*)c)->GetCacheSize();

        //////////////////////////////////// check CAT settings
#else

        n->UpdateL3CATCoreCOS();
        //n->PrintAllDataPathsInSubtree();
        long long available_L3_size = t->GetCATAwareL3Size();
#endif
        ////////////////////////////////////

        cout << "core " << myCpu << ": available L3 cache size " << available_L3_size << endl;

        tile_sz_bytes = available_L3_size * 0.9 / 3 ; //3 martices, do not go over 90 % cache occupancy

    }



    //A*B=C
    A = new double[m_dim*n_dim];//columns * rows
    B = new double[l_dim*m_dim];//columns * rows, but will be stored column-major
    C = new double[l_dim*n_dim];//columns * rows

    fill_matrices(m_dim, n_dim, l_dim);

    // print_matrix(A, m_dim, n_dim);
    // print_matrix(B, l_dim, m_dim);
    // print_matrix(C, l_dim, n_dim);

    const int tile_num_elems = tile_sz_bytes/sizeof(double);
    const int tl_sz = sqrt(tile_num_elems);

    // for (int m = 0; m < m_dim; m++) {
    //     for (int n = 0; n < n_dim; n++) {
    //         for (int l = 0; l < l_dim; l++) {
    //             C[n * l_dim + l] += A[n * m_dim + m] * B[m * l_dim + l];
    //         }
    //     }
    // }
    // print_matrix(C, l_dim, n_dim);
    // fill_matrices();

    cout << "tile dim sz: " << tl_sz << ", tile_sz_bytes: " << tile_sz_bytes << endl;
    // cout << "m_dim/tl_sz" << m_dim/tl_sz << " n_dim/tl_sz" << n_dim/tl_sz << " l_dim/tl_sz" << l_dim/tl_sz << endl;


    t_start = high_resolution_clock::now();
    for (int m = 0; m < m_dim; m += tl_sz) {
        //cout << "m" << m << endl;
        for (int n = 0; n < n_dim; n += tl_sz) {
            //cout << "  m" << m << " n" << n << endl;
            for (int l = 0; l < l_dim; l += tl_sz) {
                //cout << "    m" << m << " n" << n << " l" << l << endl;
                for (int mt = m; mt < m + tl_sz && mt < m_dim; mt++) {
                    //cout << "      m" << m << " n" << n << " l" << l << " mt" << mt << endl;
                    for (int nt = n; nt < n + tl_sz && nt < n_dim; nt++) {
                        //cout << "        m" << m << " n" << n << " l" << l << " mt" << mt << " nt" << nt << endl;
                        for (int lt = l; lt < l + tl_sz && lt < l_dim; lt++) {
                            //cout << "=         m" << m << " n" << n << " l" << l << " mt" << mt << " nt" << nt << " lt" << lt << endl;
                            C[nt * l_dim + lt] += A[nt * m_dim + mt] * B[mt * l_dim + lt];
                        }
                    }
                }
            }
        }
    }
    t_end = high_resolution_clock::now();
    cout << "time[us]: " << (t_end.time_since_epoch().count()-t_start.time_since_epoch().count())/1000 << endl;

    // print_matrix(C, l_dim, n_dim);


    return 0;
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


int hwloc_dump_xml(const char *filename)
{
    int err;
    unsigned long flags = 0; // don't show anything special
    hwloc_topology_t topology;

    err = hwloc_topology_init(&topology);
    if(err){
        std::cerr << "hwloc: Failed to initialize" << std::endl;return 1;
    }
    err = hwloc_topology_set_flags(topology, flags);
    if(err){
        std::cerr << "hwloc: Failed to set flags" << std::endl;return 1;
    }
    err = hwloc_topology_load (topology);
    if(err){
        std::cerr << "hwloc: Failed to load topology" << std::endl;return 1;
    }
    err = hwloc_topology_export_xml(topology, filename, flags);
    if(err){
        std::cerr << "hwloc: Failed to export xml" << std::endl; return 1;
    }
    return 0;
}

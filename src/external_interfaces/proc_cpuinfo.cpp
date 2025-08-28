#ifndef PROC_CPUINFO_CPP
#define PROC_CPUINFO_CPP

#include "defines.hpp"
#ifdef PROC_CPUINFO

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <tuple>
#include <chrono>

#include "Component.hpp"
#include "Thread.hpp"
#include "Core.hpp"
#include "Node.hpp"
#include "Chip.hpp"

using std::cout;
using std::endl;

//retrieve frequency in MHz from /proc/cpuinfo for each thread in std::vector<Thread*> threads
//helper function is called by RefreshCpuCoreFrequency/RefreshFreq methods
int _readCpuinfoFreq(std::vector<sys_sage::Thread*> threads, bool keep_history = false)
{
    int fd = open("/proc/cpuinfo", O_RDONLY);
    if(fd == -1)
        return -1;

    /* Advise the kernel of our access pattern.  */
    posix_fadvise(fd, 0, 0, 1);  // FDADVICE_SEQUENTIAL

    static const auto BUFFER_SIZE = 1024*16;
    char buf[BUFFER_SIZE + 1];
    buf[BUFFER_SIZE] = '\0';
    std::stringstream file;
    while(size_t bytes_read = read(fd, buf, BUFFER_SIZE))
    {
        if (!bytes_read)
            break;
        buf[bytes_read] = '\0';
        file << buf;
    }

    int num_threads = threads.size();
    std::vector<int> threadIds(num_threads);
    for(int i = 0; i<num_threads; i++)
        threadIds[i] = threads[i]->GetId();

    ptrdiff_t current_thread_pos = -1;
    int threads_processed = 0;

    std::string line;
    size_t pos = 0;
    while (std::getline(file, line))
    {
        if (line.rfind("processor", 0) == 0)
        {
            if((pos = line.find(":")) != std::string::npos)
            {
                int current_thread = stoi(line.substr(pos + 1));
                //cout << "------------Found thread " << current_thread << endl;
                current_thread_pos = find(threadIds.begin(), threadIds.end(), current_thread) - threadIds.begin();
                if(static_cast<long unsigned int>(current_thread_pos) >= threadIds.size()) {
                    current_thread_pos = -1;
                }
            }
        }
        else if (current_thread_pos != -1 && line.rfind("cpu MHz", 0) == 0)
        {
            if((pos = line.find(":")) != std::string::npos)
            {
                double freq = stod(line.substr(pos + 1));
                //find a core as a parent of this thread ID
                sys_sage::Core* c = static_cast<sys_sage::Core*>(threads[current_thread_pos]->GetAncestorByType(sys_sage::ComponentType::Core));
                if(c != NULL)
                {
                    static_cast<sys_sage::Core*>(c)->SetFreq(freq);
                    if(keep_history)
                    {
                        //check if freq_history exists; if not, create it -- vector of tuples <timestamp,frequency>
                        if (c->attrib.find("freq_history") == c->attrib.end()) {
                            c->attrib["freq_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<long long,double>>());
                        }
                        long long ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                        static_cast<std::vector<std::tuple<long long,double>>*>(c->attrib["freq_history"])->push_back(std::make_tuple(ts,freq));
                    }
                    //cout << "----------------Core " << c->GetId() << " (HW thread " << threads[current_thread_pos]->GetId() << ") frequency: " << freq << endl;
                    threads_processed++;
                    if(threads_processed == num_threads)
                    {
                        close(fd);
                        return 0;
                    }
                    current_thread_pos = -1;
                }
            }
        }
    }
    cout << "Not all cores updated their Frequency: " << threads_processed << " of total " << num_threads << " processed." << endl;
    close(fd);
    return 1;
}

int sys_sage::Node::RefreshCpuCoreFrequency(bool keep_history)
{
    std::vector<Component*> sockets = this->GetAllChildrenByType(ComponentType::Chip);
    std::vector<Thread*> cpu_hw_threads, hw_threads_to_refresh;
    for(Component * socket : sockets)
    {
        if(static_cast<Chip*>(socket)->GetChipType() == ChipType::CpuSocket || static_cast<Chip*>(socket)->GetChipType() == ChipType::Cpu)
            socket->GetAllSubcomponentsByType(reinterpret_cast<std::vector<Component*>*>(&cpu_hw_threads), ComponentType::Thread);
    }

    //remove duplicate threads of the same core (hyperthreading -- 2 threads on the same core have the same freq)
    std::set<Core*> included_cores;
    //cout << "Will check threads (cores): ";
    for(Thread* t : cpu_hw_threads){
        Core* c = static_cast<Core*>(t->GetAncestorByType(sys_sage::ComponentType::Core));
        if(included_cores.find(c) == included_cores.end())
        {
            included_cores.insert(c);
            hw_threads_to_refresh.push_back(t);
            //cout << t->GetId() << "(" << c->GetId() << "), ";
        }
    }
    //cout << endl;

    return _readCpuinfoFreq(hw_threads_to_refresh, keep_history);
}

int sys_sage::Core::RefreshFreq(bool keep_history)
{
    std::vector<Thread*> cpu_hw_threads;
    Thread* hw_thread = static_cast<Thread*>(this->GetChildByType(sys_sage::ComponentType::Thread));
    if(hw_thread != NULL)
        cpu_hw_threads.push_back(hw_thread);
    return _readCpuinfoFreq(cpu_hw_threads, keep_history);
}

int sys_sage::Thread::RefreshFreq(bool keep_history)
{
    std::vector<Thread*> cpu_hw_threads;
    cpu_hw_threads.push_back(this);
    return _readCpuinfoFreq(cpu_hw_threads, keep_history);
}

double sys_sage::Core::GetFreq() const {return freq;}
void sys_sage::Core::SetFreq(double _freq) {freq = _freq;}
double sys_sage::Thread::GetFreq()
{
    Core * c = static_cast<Core*>(this->GetAncestorByType(sys_sage::ComponentType::Core));
    if(c == NULL)
        return -1;
    return c->GetFreq();
}

#endif //PROC_CPUINFO
#endif //PROC_CPUINFO_CPP

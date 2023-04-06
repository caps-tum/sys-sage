
#ifdef CPUINFO

#include "Topology.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <tuple>
#include <chrono>


//retrieve frequency in MHz from /proc/cpuinfo for each thread in vector<Thread*> threads
//helper function is called by RefreshCpuCoreFrequency/RefreshFreq methods
int readCpuinfoFreq(std::vector<Thread*> threads, bool keep_history = false)
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

    string line;
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
                if((long unsigned int)current_thread_pos >= threadIds.size()) {
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
                Core* c = (Core*)threads[current_thread_pos]->FindParentByType(SYS_SAGE_COMPONENT_CORE);
                if(c != NULL)
                {
                    ((Core*)c)->SetFreq(freq);
                    if(keep_history)
                    {
                        //check if freq_history exists; if not, create it -- vector of tuples <timestamp,frequency>
                        if (c->attrib.find("freq_history") == c->attrib.end()) {
                            c->attrib["freq_history"] = (void*) new std::vector<std::tuple<long long,double>>();
                        }
                        long long ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                        ((std::vector<std::tuple<long long,double>>*)c->attrib["freq_history"])->push_back(std::make_tuple(ts,freq));
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

int Node::RefreshCpuCoreFrequency(bool keep_history)
{
    vector<Component*> sockets = this->GetAllChildrenByType(SYS_SAGE_COMPONENT_CHIP);
    vector<Thread*> cpu_hw_threads, hw_threads_to_refresh;
    for(Component * socket : sockets)
    {
        if(((Chip*)socket)->GetChipType() == SYS_SAGE_CHIP_TYPE_CPU_SOCKET || ((Chip*)socket)->GetChipType() == SYS_SAGE_CHIP_TYPE_CPU)
            socket->FindAllSubcomponentsByType((vector<Component*>*)&cpu_hw_threads, SYS_SAGE_COMPONENT_THREAD);
    }

    //remove duplicate threads of the same core (hyperthreading -- 2 threads on the same core have the same freq)
    std::set<Core*> included_cores;
    //cout << "Will check threads (cores): ";
    for(Thread* t : cpu_hw_threads){
        Core* c = (Core*)t->FindParentByType(SYS_SAGE_COMPONENT_CORE);
        if(included_cores.find(c) == included_cores.end())
        {
            included_cores.insert(c);
            hw_threads_to_refresh.push_back(t);
            //cout << t->GetId() << "(" << c->GetId() << "), ";
        }
    }
    //cout << endl;

    return readCpuinfoFreq(hw_threads_to_refresh, keep_history);
}

int Core::RefreshFreq(bool keep_history)
{
    vector<Thread*> cpu_hw_threads;
    Thread* hw_thread = (Thread*)this->GetChildByType(SYS_SAGE_COMPONENT_THREAD);
    if(hw_thread != NULL)
        cpu_hw_threads.push_back(hw_thread);
    return readCpuinfoFreq(cpu_hw_threads, keep_history);
}

int Thread::RefreshFreq(bool keep_history)
{
    vector<Thread*> cpu_hw_threads;
    cpu_hw_threads.push_back(this);
    return readCpuinfoFreq(cpu_hw_threads, keep_history);
}

double Core::GetFreq() {return freq;}
void Core::SetFreq(double _freq) {freq = _freq;}
double Thread::GetFreq()
{
    Core * c = (Core*)this->FindParentByType(SYS_SAGE_COMPONENT_CORE);
    if(c == NULL)
        return -1;
    return c->GetFreq();
}
#endif

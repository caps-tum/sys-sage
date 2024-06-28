#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>

DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type){
    return NewDataPath(_source,_target,_oriented,_type,(double)-1,(double)-1);
}
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency){
    return NewDataPath(_source,_target,_oriented,SYS_SAGE_DATAPATH_TYPE_NONE,_bw,_latency);
}
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency)
{
    DataPath* p = new DataPath(_source, _target, _oriented, _type, _bw, _latency);
    return p;
}

Component * DataPath::GetSource() {return source;}
Component * DataPath::GetTarget() {return target;}
double DataPath::GetBandwidth() {return bw;}
void DataPath::SetBandwidth(double _bandwidth) { bw = _bandwidth;}
double DataPath::GetLatency() {return latency;}
void DataPath::SetLatency(double _latency) { latency = _latency; }
int DataPath::GetDataPathType() {return dp_type;}
int DataPath::GetOrientation() {return oriented;}

DataPath::DataPath(Component* _source, Component* _target, int _oriented, int _type): DataPath(_source, _target, _oriented, _type, -1, -1) {}
DataPath::DataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency): DataPath(_source, _target, _oriented, SYS_SAGE_DATAPATH_TYPE_NONE, _bw, _latency) {}
DataPath::DataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency): source(_source), target(_target), oriented(_oriented), dp_type(_type), bw(_bw), latency(_latency)
{
    if(_oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL)
    {
        _source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
        _target->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
        _source->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
        _target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    }
    else if(_oriented == SYS_SAGE_DATAPATH_ORIENTED)
    {
        _source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
        _target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    }
    else
    {
        delete this;
        return;//error
    }
}

void DataPath::DeleteDataPath()
{
    if(oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL)
    {
        std::vector<DataPath*>* source_dp_outgoing = source->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        std::vector<DataPath*>* source_dp_incoming = source->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
        std::vector<DataPath*>* target_dp_outgoing = target->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        std::vector<DataPath*>* target_dp_incoming = target->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);

        source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
        target_dp_outgoing->erase(std::remove(target_dp_outgoing->begin(), target_dp_outgoing->end(), this), target_dp_outgoing->end());
        source_dp_incoming->erase(std::remove(source_dp_incoming->begin(), source_dp_incoming->end(), this), source_dp_incoming->end());
        target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
    }
    else if(oriented == SYS_SAGE_DATAPATH_ORIENTED)
    {
        std::vector<DataPath*>* source_dp_outgoing = source->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        std::vector<DataPath*>* target_dp_incoming = target->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
        source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
        target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
    }
    delete this;
}


void DataPath::Print()
{
    cout << "DataPath src: (" << source->GetComponentTypeStr() << ") id " << source->GetId() << ", target: (" << target->GetComponentTypeStr() << ") id " << target->GetId() << " - bw: " << bw << ", latency: " << latency;
    if(!attrib.empty())
    {
        cout << " - attrib: ";
        for (const auto& n : attrib) {
            uint64_t* val = (uint64_t*)n.second;
            std::cout << n.first << " = " << *val << "; ";
        }
    }
    cout << endl;
}

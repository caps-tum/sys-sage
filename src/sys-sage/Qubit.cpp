#include "Qubit.hpp"



sys_sage::Qubit::Qubit(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Qubit){}
sys_sage::Qubit::Qubit(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Qubit){}

//SVTODO maybe this is what the constructor in xml_load needs?
void sys_sage::Qubit::SetProperties(double _t1, double _t2, double _readout_fidelity, double _q1_fidelity, double _readout_length)
{
    t1 = _t1;
    t2 = _t2;
    readout_fidelity = _readout_fidelity;
    q1_fidelity = _q1_fidelity;
    readout_length = _readout_length;

}

double sys_sage::Qubit::GetT1() const { return t1; }    
double sys_sage::Qubit::GetT2() const { return t2; }
double sys_sage::Qubit::GetReadoutFidelity() const { return readout_fidelity; }
double sys_sage::Qubit::Get1QFidelity() const { return q1_fidelity;}
double sys_sage::Qubit::GetReadoutLength() const { return readout_length; }
double sys_sage::Qubit::GetFrequency() const { return frequency; }
const std::string& sys_sage::Qubit::GetCalibrationTime() const { return calibration_time; }

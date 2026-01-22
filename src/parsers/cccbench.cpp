#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <string>
#include <vector>
#include <exception>
//#include <bits/stdc++.h>
#include <sstream>
#include "cccbench.hpp"

using namespace std;

sys_sage::CccbenchParser::CccbenchParser(const char *csv_path)
    : c2cDatapoints((Vec2DArray<float> *)0)
{
    fstream fs;
    string line, token;
    vector <std::string>ltokens;
    vector <std::string>columns;
    int i=0, metric_i=-1, xcore_i=-1, ycore_i=-1, elements_per_line = -1;

    //c2cDatapoints;
    this->firstCore = INT_MAX;
    this->lastCore = 0;
    fs.open(csv_path, ios::in);
    if(!fs.is_open())
    {
        //throw std::runtime_error();
        throw "failed to open file";
    }
    while(getline(fs, line))
    {
        if(line.empty())// || 
           //std::all_of(line.begin(), line.end(), [](char c){return std::isspace(c);}))
            continue; //allow and discard empty lines
        int within_line_i = 0;
        unsigned int tok_int=0;
        std::stringstream linestream(line);
        if(0 == i++)
        {
            while(std::getline(linestream, token, ','))
            {
                columns.push_back(token);
                if(token == this->metric_name)
                    metric_i = within_line_i;
                if(token == this->xcore_name)
                    xcore_i = within_line_i;
                if(token == this->ycore_name)
                    ycore_i = within_line_i;
                within_line_i++;
            }
            elements_per_line = within_line_i;
            continue;
        }
        //assertions used for things related to the expected data source format
        assert(xcore_i > -1);
        assert(ycore_i > -1);
        assert(metric_i > -1);
        while(std::getline(linestream, token, ','))
        {
            //assuming x and y are in the same range (all to all) 
            if(xcore_i == within_line_i || ycore_i == within_line_i)
            {
                tok_int = std::stoi(token);
                this->firstCore = (this->firstCore > tok_int) ? tok_int: this->firstCore;
                this->lastCore = (this->lastCore < tok_int) ? tok_int: this->lastCore;
            }
            ltokens.push_back(token);
            within_line_i++;
        }
        assert(within_line_i <= elements_per_line);
    }
    fs.close();
    this->lines = i-1; //ignore header line
    int dimension = 1 + this->lastCore - this->firstCore;
    this->c2cDatapoints = new Vec2DArray<float>(dimension, dimension);
    for(unsigned int i=0; i<lines; ++i)
    {
        auto xi = std::stoi(ltokens[xcore_i + i*elements_per_line]) - this->firstCore;
        auto yi = std::stoi(ltokens[ycore_i + i*elements_per_line]) - this->firstCore;
        (*this->c2cDatapoints)[xi][yi].push_back(std::stof(ltokens[metric_i + i*elements_per_line]));
    }
}

void sys_sage::CccbenchParser::applyDataPaths(Component *root)
{
    auto corev = new vector<Component *>();
    root->FindDescendantsByType(corev, sys_sage::ComponentType::Core);
    //auto corev = root->GetAllChildrenByType(SYS_SAGE_COMPONENT_CORE);

    for(auto xcore : *corev)
    {
        for(auto ycore : *corev)
        {
            auto xci = xcore->GetId();
            auto yci = ycore->GetId();
            if(xci == yci)
            {
                continue;
            }
            auto xtoylatv = (*this->c2cDatapoints)[xci][yci];
            auto sum = accumulate(xtoylatv.begin(), xtoylatv.end(), 0.0);
            auto mean = new float(sum / xtoylatv.size());
            auto max = new float(*max_element(xtoylatv.begin(), xtoylatv.end()));
            auto min = new float(*min_element(xtoylatv.begin(), xtoylatv.end()));
            auto dtp = new DataPath(xcore, ycore, sys_sage::DataPathOrientation::Oriented,
                                   sys_sage::DataPathType::C2C, 0, *mean);
            dtp->attrib.insert(std::pair<string, void *>("latency_max", reinterpret_cast<void*>(max)));
            dtp->attrib.insert(std::pair<string, void *>("latency_min", reinterpret_cast<void*>(min)));
            dtp->attrib.insert(std::pair<string, void *>("latency", reinterpret_cast<void*>(mean)));
        }
    }
}

int sys_sage::parseCccbenchOutput(Node* n, std::string cccPath)
{
    const char *cstr_path = cccPath.c_str();
    auto cccparser = new CccbenchParser(cstr_path);
    cccparser->applyDataPaths(n);
    delete cccparser;
    return 0;
}



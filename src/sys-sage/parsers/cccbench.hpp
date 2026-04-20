#ifndef CCCBENCH_PARSER
#define CCCBENCH_PARSER

#include <vector>

#include "enums.hpp"

#include "Component.hpp"
#include "Node.hpp"




namespace sys_sage {
    int parseCccbenchOutput(Node* , std::string );

    template <typename T>class Vec2DArray
    {
        Vec2DArray(){}
        std::vector<T> *array;
        unsigned int size, xdim, ydim;
    public:
        Vec2DArray(unsigned xdim, unsigned ydim);
        std::vector<T> *operator [](unsigned int xindex);
    };

    template <typename T>Vec2DArray<T>::Vec2DArray(unsigned int _xdim, unsigned int _ydim)
    {
        this->size = _xdim * _ydim;
        this->xdim = _xdim;
        this->ydim = _ydim;
        this->array = new std::vector<T> [this->size];
    }

    template <typename T>std::vector<T> *Vec2DArray<T>::operator [](unsigned int xindex)
    {
        return &(this->array[this->xdim*xindex]);
    }

    class CccbenchParser{
        unsigned int firstCore;
        unsigned int lastCore;
        unsigned int lines;
        const char *metric_name = "xylat";
        const char *xcore_name = "xcore";
        const char *ycore_name = "ycore";
        Vec2DArray<float> *c2cDatapoints;
        CccbenchParser():c2cDatapoints((Vec2DArray<float> *)0){}
    public:
        virtual ~CccbenchParser(){if(this->c2cDatapoints) {delete [] c2cDatapoints;}}
        unsigned int xtoi(unsigned int _x){return _x - this->firstCore;}
        unsigned int ytoi(unsigned int _y){return _y - this->firstCore;}
        CccbenchParser(const char *csv_path);
        void applyDataPaths(Component *root);
    };

} //namespace sys_sage
#endif

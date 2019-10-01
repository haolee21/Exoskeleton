#include "BWFilter.hpp"
#include <cstring>

BWFilter::BWFilter(/* args */)
{
    shared_ptr<unsigned int[]> *curHead_i = this->bufList;
    while(curHead_i != end(this->bufList)){
        curHead_i->reset(new unsigned int[NUMSEN]);
        memset(curHead_i->get(),0,NUMSEN*sizeof(unsigned int));
    }
    shared_ptr<float[]> *curHead_o = this->outBufList;
    while(curHead_o!=end(this->outBufList)){
        curHead_o->reset(new float [NUMSEN]);
        memset(curHead_o->get(),0,NUMSEN*sizeof(float));
    }
}

BWFilter::~BWFilter()
{
}

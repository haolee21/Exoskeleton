#include "BWFilter.hpp"
#include <cstring>

BWFilter::BWFilter()
{
    


    this->buf1.reset(new unsigned int[NUMSEN]);
    memset(this->buf1.get(),0,NUMSEN*sizeof(unsigned int));
    this->buf2.reset(new unsigned int[NUMSEN]);
    memset(this->buf2.get(),0,NUMSEN*sizeof(unsigned int));
    this->buf3.reset(new unsigned int[NUMSEN]);
    memset(this->buf3.get(),0,NUMSEN*sizeof(unsigned int));

    this->outBuf0.reset(new float[NUMSEN]);
    memset(this->outBuf0.get(),0.0,NUMSEN*sizeof(float));
    this->outBuf1.reset(new float[NUMSEN]);
    memset(this->outBuf1.get(),0.0,NUMSEN*sizeof(float));
    this->outBuf2.reset(new float[NUMSEN]);
    memset(this->outBuf2.get(),0.0,NUMSEN*sizeof(float));
    this->outBuf3.reset(new float[NUMSEN]);
    memset(this->outBuf3.get(),0.0,NUMSEN*sizeof(float));
}

BWFilter::~BWFilter()
{
}

void BWFilter::FilterData(unsigned int *newMea,unsigned int *output){
    // attention, the output array is the senData in sensor, since the senData[0] is reserved for time
    // recording sensing data need to start at 1

    for(int i=0;i<NUMSEN;i++){
        
        float curSen = this->b0*newMea[i]+this->b1*this->buf1[i]+this->b2*this->buf2[i]+this->b3*this->buf3[i]-this->a1*this->outBuf2[i]-this->a2*this->outBuf2[i]-this->a3*this->outBuf3[i];
        output[i+1]=curSen;
        this->outBuf0[i] = curSen;
    }

    this->outBuf3.swap(this->outBuf2);
    this->outBuf2.swap(this->outBuf1);
    this->outBuf1.swap(this->outBuf0);

    this->buf3.swap(this->buf2);
    this->buf2.swap(this->buf1);
    memcpy(this->buf1.get(),newMea,NUMSEN*sizeof(unsigned int));
    
}
bool BWFilter::InitBuffer(unsigned int *newMea){
    memcpy(this->buf1.get(),newMea,NUMSEN*sizeof(unsigned int));
    for(int i=0;i<NUMSEN;i++){
        this->outBuf0[i] = (float)newMea[i];
        this->outBuf1[i] = (float)newMea[i];
        this->outBuf2[i] = (float)newMea[i];
        this->outBuf3[i] = (float)newMea[i];
    }
    return true;
}

#include "common.hpp"
#include<mutex>
#include <iostream>
#define SENBUFFSIZE (DATALEN*50)
template<class T>
class SenBuffer
{
private:
    T buffer[SENBUFFSIZE];
    int endIdx;
    int readUntil_idx;
    int totalByte;
    
    
public:
    SenBuffer(/* args */);
    ~SenBuffer();
    void PushData(T *inData, int numByte);
    bool GetData(T *returnBuf);
};
template<class T>
SenBuffer<T>::SenBuffer(/* args */)
{
    this->endIdx = 0;
    this->readUntil_idx = 0;
    this->totalByte = 0;
    
}
template<class T>
SenBuffer<T>::~SenBuffer()
{
}
template<class T>
void SenBuffer<T>::PushData(T *inData, int numByte){
    int overFlowByte = SENBUFFSIZE - this->endIdx - numByte;
    this->totalByte += numByte;
    if (overFlowByte < 0)
    {
        std::copy(inData, inData + numByte + overFlowByte, this->buffer+this->endIdx);
        std::copy(inData + numByte + overFlowByte, inData + numByte, this->buffer);
        this->endIdx = -overFlowByte;
    }
    else{
        std::copy(inData, inData + numByte, this->buffer+this->endIdx);
        this->endIdx += numByte;
    }
}
template<class T>
bool SenBuffer<T>::GetData(T *returnBuf){
    bool enoughData = false;
    
        
    if(this->totalByte>=DATALEN){
        enoughData = true;
        this->totalByte -= DATALEN;
    }

    if(enoughData){
        int overFlowByte = SENBUFFSIZE - this->readUntil_idx - DATALEN;
        if (overFlowByte<0){
            std::copy(this->buffer + this->readUntil_idx, this->buffer + this->readUntil_idx + overFlowByte+DATALEN, returnBuf);
            std::copy(this->buffer, this->buffer - overFlowByte, returnBuf + overFlowByte+DATALEN);
            this->readUntil_idx = -overFlowByte;
        }
        else{
            std::copy(this->buffer + this->readUntil_idx, this->buffer + this->readUntil_idx + DATALEN, returnBuf);
            this->readUntil_idx += DATALEN;
        }
        
    }
    else{
        
    }
    return enoughData;
}
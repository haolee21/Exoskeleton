#include "common.hpp"
#define SENBUFFSIZE (DATALEN*150+1)
#include <iostream>
class SenBuffer
{
private:
    char buffer[SENBUFFSIZE];
    int endIdx;
    int readUntil_idx;
    int totalByte;
    bool findEnd;
    
public:
    SenBuffer(/* args */);
    ~SenBuffer();
    void PushData(char *inData, int numByte);
    bool GetData(char *returnBuf);
};


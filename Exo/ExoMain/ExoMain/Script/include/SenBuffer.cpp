#include"SenBuffer.hpp"
SenBuffer::SenBuffer(/* args */)
{
    this->endIdx = 0;
    this->readUntil_idx = 0;
    this->totalByte = 0;
    this->findEnd = false;
    
}

SenBuffer::~SenBuffer()
{
}
void SenBuffer::PushData(char *inData, int numByte){
    int overFlowByte = SENBUFFSIZE - this->endIdx - numByte;
    this->totalByte += numByte;
    if (overFlowByte < 0)
    {
        if(numByte+overFlowByte>0)
            std::copy(inData, inData + numByte + overFlowByte, this->buffer+this->endIdx);
        std::copy(inData + numByte + overFlowByte, inData + numByte, this->buffer);
        this->endIdx = -overFlowByte;
    }
    else{
        std::copy(inData, inData + numByte, this->buffer+this->endIdx);
        this->endIdx += numByte;
    }
}
bool SenBuffer::GetData(char *returnBuf){
    if(!this->findEnd){
        while(this->totalByte>0){
            if(this->buffer[this->readUntil_idx]!='\n'){
                this->readUntil_idx++;
                this->totalByte--;
                if(this->readUntil_idx==SENBUFFSIZE){
                    this->readUntil_idx =0;
                }
            }
            else{
                this->findEnd = true;
                this->readUntil_idx++;
                this->totalByte--;
                break;
            }
        }
    }
    bool enoughData = false;    
    if(this->totalByte>=DATALEN){
        enoughData = true;
        this->totalByte -= DATALEN;
    }

    
    if(enoughData){
        int overFlowByte = SENBUFFSIZE - this->readUntil_idx - DATALEN;
        if (overFlowByte<0){
            if(this->readUntil_idx+overFlowByte+DATALEN>0)
                std::copy(this->buffer + this->readUntil_idx, this->buffer + this->readUntil_idx + overFlowByte+DATALEN, returnBuf);
            std::copy(this->buffer, this->buffer - overFlowByte, returnBuf + overFlowByte+DATALEN);
            this->readUntil_idx = -overFlowByte;
        }
        else{
            std::copy(this->buffer + this->readUntil_idx, this->buffer + this->readUntil_idx + DATALEN, returnBuf);
            this->readUntil_idx += DATALEN;
        }
        if(returnBuf[DATALEN-1]!='\n'){
            enoughData = false;
            findEnd = false;
        }
        
    }
    
    return enoughData;
}
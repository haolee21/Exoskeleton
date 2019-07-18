#ifndef RECORDER_HPP
#define RECORDER_HPP

#include <string>
#include <queue>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdio>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include "RecData.hpp"
#define MAXRECLENGTH 5000

struct RecDataTemp
{
    int test;
};

class Recorder
{
private:
    RecData<int> *curSenData;
    std::queue<std::string> senTemps;
    int tempSenCount;
    RecData<bool> *curValCondData;
    std::queue<std::string> valCondTemps;
    int tempValCondCount;
    // this is for creating temperary object for saving, I am not sure do I really need it
    RecData<int> *tempSenData;
    RecData<bool> *tempValCondData;
    
    std::string senLabel;
    std::string valveLabel;
    std::string recorderName;


    int tempFilesCount;
    template<class T>
    void writeTemp(RecData<T> *tempData,int recType); //0: sensor, 1:valveCond

    // saving file
    std::string filePath;
    std::queue<std::thread*> threadQue;

public:
    Recorder(std::string recName, std::string senLabel,std::string valLabel);
    ~Recorder();
    void PushSen(unsigned long curTime, std::vector<int> curSen);
    void PushValveCond(unsigned long curTime, std::vector<bool> curValCond);
    void OutputCSV();
};

#endif
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

template<class T>
class Recorder
{
private:
    RecData<T> *curData;
    std::queue<std::string> dataTemps;
    int tempDataCount;
    // this is for creating temperary object for saving, I am not sure do I really need it
    std::string label;
    std::string recorderName;

    int tempFilesCount;
    
    void writeTemp(RecData<T> *tempData); //0: sensor, 1:valveCond

    // saving file
    std::string filePath;
    std::queue<std::thread*> threadQue;

public:
    Recorder(std::string recName, std::string label);
    ~Recorder();
   
    void PushData(unsigned long curTime, std::vector<T> curSen);
    void OutputCSV();
};

#endif

#ifndef RECORDER_HPP
#define RECORDER_HPP
#include <memory>
#include <string>
#include <queue>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdio>
#include <sstream>
#include <iterator>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

#include "RecData.hpp"

#define MAXRECLENGTH 20000


template<class T>
class Recorder
{
private:
    std::unique_ptr<RecData<T>> curData;
    // RecData<T> *curData;
    std::queue<std::string> dataTemps;
    int tempCount;
    std::unique_ptr<RecData<T>> test;
    // this is for creating temperary object for saving, I am not sure do I really need it
    std::unique_ptr<RecData<T>> tempData;
    // RecData<T> *tempData;
    std::string Label;
    std::string recorderName;


    int tempFilesCount;
    
    void writeTemp(); 

    // saving file
    std::string filePath;
    std::queue<std::thread*> threadQue;

    

public:
    Recorder(std::string recName,std::string _filePath, std::string _label);
    
    ~Recorder();
    void PushData(unsigned long curTime, std::vector<T> curMea);
    void OutputCSV();
};


template<class T>
Recorder<T>::Recorder(std::string recName,std::string _filePath, std::string _label)
{
    
    this->recorderName = recName;
    this->Label = _label;
    
    
    this->tempCount=0;
    this->filePath = _filePath;

    //create memory for temperary storage
    // since it may go out of the scope, we put it in the heap
    this->curData.reset(new RecData<T>());
    this->tempFilesCount =0;
    

}
template<class T>
Recorder<T>::~Recorder()
{
    
    while (!this->threadQue.empty())
    {
        std::thread *curThread = this->threadQue.front();
        curThread->join();
        this->threadQue.pop();
    }
    
    
    std::thread saveTh = std::thread(&Recorder::OutputCSV,this);
    //this->OutputCSV();
    saveTh.join();

    
    
    if(!this->dataTemps.empty())
        std::cout<<"Temperary files remaining\n";
    
    

}
template<class T>
void Recorder<T>::PushData(unsigned long curTime, std::vector<T> curData){
    this->curData->PushTime(curTime);
    this->curData->PushData(curData);
    if((++this->tempCount)>=MAXRECLENGTH){
        
        this->tempCount =0;
        //we need to allocate new memory for storage
        this->tempData.swap(this->curData);
        this->curData.reset(new RecData<T>());
        this->threadQue.push(new std::thread(&Recorder<T>::writeTemp,this));   
    }
}

template<class T>
void Recorder<T>::writeTemp()
{
    std::string fileName;
    fileName = this->filePath+ this->recorderName + std::to_string(this->tempFilesCount++) + ".temp";
    this->dataTemps.push(fileName);
    {
        std::ofstream tempSaveFile(fileName);
        boost::archive::text_oarchive ar(tempSaveFile);
        ar& (*this->tempData);
        
    }
    
}
template<class T>
void Recorder<T>::OutputCSV()
{
    // create senData.csv
    {
        std::ofstream writeCsv;
        std::ostringstream vts;
        writeCsv.open(this->filePath+"/"+this->recorderName+".csv");
        {
            vts<<this->Label<<'\n';
            //writeCsv<<this->Label<<'\n';
            while(!this->dataTemps.empty()){
                
                RecData<T> tempData = RecData<T>();
                std::string fileName = this->dataTemps.front();
                std::ifstream ifs(fileName);
                boost::archive::text_iarchive ar(ifs);
                this->dataTemps.pop();
                ar & tempData;
                std::vector<std::vector<T>> data = tempData.getData();
                std::vector<unsigned long> time = tempData.getTime();
                
                

                for(unsigned int i=0;i<MAXRECLENGTH;i++){
                    vts<<std::to_string(time[i])<<",";
                    std::vector<T> curRow = data[i];
                    std::copy(curRow.begin(),curRow.end()-1,std::ostream_iterator<T>(vts,","));
                    vts<<curRow.back()<<'\n';
                }
                
                std::remove(&fileName[0]);
            }
            
            // save data has not long enough to be in temp
            std::vector<std::vector<T>> leftData = this->curData->getData();
            std::vector<unsigned long> leftTime = this->curData->getTime();
            for(int i =0;i<this->tempCount;i++){
                std::vector<T> curRow = leftData[i];
                vts<<std::to_string(leftTime[i])<<",";

                std::copy(curRow.begin(),curRow.end()-1,std::ostream_iterator<T>(vts,","));
                vts<<curRow.back()<<'\n';
            }
        }
        writeCsv<<vts.str();
        writeCsv.close();
        
    }


}
#endif
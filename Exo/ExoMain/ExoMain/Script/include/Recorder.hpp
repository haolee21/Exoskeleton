#ifndef RECORDER_HPP
#define RECORDER_HPP
#include "RecData.hpp"
#define CHECK_TH_SLEEP (MAX_REC_LEN) //based on 1k Hz, check every half filled time
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <memory>
#include <thread>
//timer
#include <unistd.h>
#include <iostream>
template<class T,std::size_t N>
class Recorder
{
    
private:
    std::string labels;
    std::string recName;
    std::string filePath;
    int tempFileCount;
    


    std::unique_ptr<RecData<T,N>> curRecData;
    std::queue<std::thread*> saveThQue;
    std::queue<std::string> savedFiles;

    static void writeTemp(std::unique_ptr<RecData<T,N>> fullData,std::string fileName){
        
        
        {
            std::ofstream of(fileName);
            boost::archive::text_oarchive ar(of);
            ar <<(*fullData);
        }
        std::cout<<"finish write a temp file\n";
    }
    void saveAllTemp(){
        //save all the temp files
        while(!saveThQue.empty()){
            std::thread *saveTask = saveThQue.front();
            saveTask->join();
            saveThQue.pop();
        }
        
    }
    std::unique_ptr<std::thread> checkTh_th;
    
    
    bool checkTh_flag = true;
    void CheckThreadsLoop(){
        std::cout<<"check loop start\n";
        while(checkTh_flag || !saveThQue.empty()){
            saveAllTemp();
            
            usleep(CHECK_TH_SLEEP);
        }
        std::cout<<"all threads end\n";

    }

    void output_csv(){
        std::cout<<"start to write csv\n";
        std::ofstream writeCsv;
        std::ostringstream vts;
        writeCsv.open(filePath+recName+".csv");
        vts<<labels<<'\n';
        if(savedFiles.empty()){
            std::cout<<"empty saveFiles\n";
        }
        while(!savedFiles.empty()){
            std::cout<<"some temp files\n";
            std::string fileName = savedFiles.front();
            savedFiles.pop();
            std::ifstream ifs(fileName);
            boost::archive::text_iarchive ar(ifs);

            RecData<T,N> tempRecData = RecData<T,N>();
            ar >> tempRecData;
            tempRecData.PrintData(vts,MAX_REC_LEN);

            std::remove(&fileName[0]);
        }
        // //write the not saved data
        std::cout<<"done saving temp files\n";
        curRecData->PrintData(vts,curRecData->getLen());
        writeCsv<<vts.str();
        writeCsv.close();
        
    }
public:
    void PushData(unsigned long curTime, std::array<T,N>cur_data){
        if(!curRecData->PushData(curTime,cur_data)){
            std::unique_ptr<RecData<T,N>> fullRecData (new RecData<T,N>()); //create new recData when it is full
            curRecData.swap(fullRecData);//swap with current data

            std::string fileName = filePath+recName+std::to_string(tempFileCount++) + ".temp";
            savedFiles.push(fileName);
            saveThQue.push(new std::thread(&Recorder::writeTemp,std::move(fullRecData),fileName));
            
        }
    }
    Recorder(std::string _recName, std::string _filePath, std::string _labels){
        recName = _recName;
        labels = _labels;
        filePath = _filePath;
        tempFileCount=0;
        curRecData.reset(new RecData<T,N>());
        checkTh_th.reset(new std::thread(&Recorder::CheckThreadsLoop,this));
        
    }
    void End(){
        std::cout<<"called end function\n";
        
        checkTh_flag = false;
        if(checkTh_th->joinable())
            checkTh_th->join();
        std::cout<<"done join\n";
        output_csv();
        

    }
    ~Recorder(){
        End();
        
    }
};

#endif //RECORDER_HPP
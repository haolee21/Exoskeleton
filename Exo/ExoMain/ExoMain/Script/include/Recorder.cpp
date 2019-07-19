#include "Recorder.hpp"

Recorder::Recorder(std::string recName,std::string senLabel,std::string valveLabel)
{
    std::cout<<"create\n";
    this->recorderName = recName;
    this->senLabel = senLabel;
    this->valveLabel = valveLabel;
    
    this->tempSenCount=0;
    this->tempValCondCount = 0;
    this->filePath = "";

    //create memory for temperary storage
    // since it may go out of the scope, we put it in the heap
    this->curSenData = new RecData<int>();
    this->curValCondData = new RecData<bool>();
    this->tempFilesCount =0;
    
}

Recorder::~Recorder()
{
    while (!this->threadQue.empty())
    {
        std::thread *curThread = this->threadQue.front();
        curThread->join();
        delete curThread;
        this->threadQue.pop();
    }
    
    this->OutputCSV();

    if(!this->senTemps.empty())
        std::cout<<"Temperary files remaining\n";
    delete this->curSenData;
    delete this->curValCondData;
}
void Recorder::PushSen(unsigned long curTime, std::vector<int> curSen){
    this->curSenData->PushTime(curTime);
    this->curSenData->PushData(curSen);
    if((++this->tempSenCount)>=MAXRECLENGTH){
        this->tempSenCount =0;
        //we need to allocate new memory for storage
        this->tempSenData = this->curSenData;
        this->curSenData = new RecData<int>();
        this->threadQue.push(new std::thread(&Recorder::writeTemp<int>,this,this->tempSenData,0));   
    }
}
void Recorder::PushValveCond(unsigned long curTime, std::vector<bool> curValCond){
    this->curValCondData->PushTime(curTime);
    this->curValCondData->PushData(curValCond);
    
    if((++this->tempValCondCount)>=MAXRECLENGTH){
        
        this->tempValCondCount=0;
        //we need to allocate new memory for storage
        this->tempValCondData = this->curValCondData;
        this->curValCondData = new RecData<bool>();
        this->threadQue.push(new std::thread(&Recorder::writeTemp<bool>,this,this->tempValCondData,1));
        
    }
    

}

template<class T>
void Recorder::writeTemp(RecData<T> *tempData, int recType)
{
    std::string fileName;
    fileName = this->filePath+ this->recorderName + std::to_string(this->tempFilesCount++) + ".temp";
    //in this file we need to record both sensor data and valve condtion
    switch (recType)
    {
    case 0:
        this->senTemps.push(fileName);
        break;
    case 1:
        this->valCondTemps.push(fileName);
    default:
        break;
    }
    {
        std::ofstream tempSaveFile(fileName);
        boost::archive::text_oarchive ar(tempSaveFile);
        ar& (*tempData);
        delete tempData;
    }
    
}
void Recorder::OutputCSV()
{
    // create senData.csv
    {
        std::ofstream writeSenCsv;
        writeSenCsv.open(this->recorderName+"_sen.csv");
        {
            writeSenCsv<<this->senLabel<<'\n';
            while(!this->senTemps.empty()){
                RecData<int> tempSen = RecData<int>();
                std::string fileName = this->senTemps.front();
                std::ifstream ifs(fileName);
                boost::archive::text_iarchive ar(ifs);
                this->senTemps.pop();
                ar & tempSen;
                std::vector<std::vector<int>> senData = tempSen.getData();
                std::vector<unsigned long> senTime = tempSen.getTime();
                for(unsigned int i=0;i<MAXRECLENGTH;i++){
                    writeSenCsv<<std::to_string(senTime[i]);
                    for(unsigned int ii=0;ii<senData[i].size();ii++){
                        writeSenCsv<<','<<senData[i][ii];
                    }
                    writeSenCsv<<'\n';
                }
                std::remove(&fileName[0]);
            }
        }
        writeSenCsv.close();
    }
    {
        std::ofstream writeConCsv;
        writeConCsv.open(this->recorderName+"_con.csv");
        {
            writeConCsv<<this->valveLabel<<'\n';
            while(!this->valCondTemps.empty()){
                RecData<bool> tempCon = RecData<bool>();
                std::string fileName = this->valCondTemps.front();
                std::ifstream ifs(fileName);
                boost::archive::text_iarchive ar(ifs);
                this->valCondTemps.pop();
                ar & tempCon;
                std::vector<std::vector<bool>> senData = tempCon.getData();
                std::vector<unsigned long> senTime = tempCon.getTime();
                for(unsigned int i=0;i<MAXRECLENGTH;i++){
                    writeConCsv<<std::to_string(senTime[i]);
                    for(unsigned int ii=0;ii<senData[i].size();ii++){
                        writeConCsv<<','<<senData[i][ii];
                    }
                    writeConCsv<<'\n';
                }
                std::remove(&fileName[0]);
            }
        }
        writeConCsv.close();
    }


}
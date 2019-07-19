#include "Recorder.hpp"
template<class T>
Recorder<T>::Recorder(std::string recName,std::string dataLabel)
{
    std::cout<<"create\n";
    this->recorderName = recName;
    this->label = dataLabel;
    
    
    this->tempDataCount=0;
    
    this->filePath = "";

    //create memory for temperary storage
    // since it may go out of the scope, we put it in the heap
    
    this->curData = new RecData<T>();
    this->tempFilesCount =0;
    
}
template<class T>
Recorder<T>::~Recorder()
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
    delete this->curData;
}
template<class T>
void Recorder<T>::PushData(unsigned long curTime, std::vector<T> curData){
    this->curSenData->PushTime(curTime);
    this->curSenData->PushData(curData);
    if((++this->tempDataCount)>=MAXRECLENGTH){
        this->tempDataCount =0;
        //we need to allocate new memory for storage
        this->tempDataData = this->curSenData;
        this->curData = new RecData<T>();
        this->threadQue.push(new std::thread(&Recorder::writeTemp,this,this->tempSenData,0));   
    }
}

template<class T>
void Recorder<T>::writeTemp(RecData<T> *tempData)
{
    std::string fileName;
    fileName = this->filePath+ this->recorderName + std::to_string(this->tempFilesCount++) + ".temp";
    //in this file we need to record both sensor data and valve condtion
    this->dataTemps.push(fileName);
    {
        std::ofstream tempSaveFile(fileName);
        boost::archive::text_oarchive ar(tempSaveFile);
        ar& (*tempData);
        delete tempData;
    }
    
}
template<class T>
void Recorder<T>::OutputCSV()
{
    // create senData.csv
    {
        std::ofstream writeCsv;
        writeCsv.open(this->recorderName+".csv");
        {
            writeCsv<<this->label<<'\n';
            while(!this->dataTemps.empty()){
                RecData<T> tempData = RecData<T>();
                std::string fileName = this->dataTemps.front();
                std::ifstream ifs(fileName);
                boost::archive::text_iarchive ar(ifs);
                this->dataTemps.pop();
                ar & tempData;
                std::vector<std::vector<T>> Data = tempData.getData();
                std::vector<unsigned long> Time = tempData.getTime();
                for(unsigned int i=0;i<MAXRECLENGTH;i++){
                    writeCsv<<std::to_string(Time[i]);
                    for(unsigned int ii=0;ii<Data[i].size();ii++){
                        writeCsv<<','<<Data[i][ii];
                    }
                    writeCsv<<'\n';
                }
                std::remove(&fileName[0]);
            }
        }
        writeCsv.close();
    }
    


}
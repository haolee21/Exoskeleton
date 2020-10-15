#ifndef RECDATA_HPP
#define RECDATA_HPP
#include <sstream>
#include <array>
#include <memory>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/array.hpp> //necessary, otherwise we cannot serialize std::array
#define MAX_REC_LEN (1000*60*2)

template<class T,std::size_t N>
class RecData
{
private:
    std::array<std::array<T,N>,MAX_REC_LEN> data;    
    std::array<unsigned long,MAX_REC_LEN> time;
    
    // Allow serialization to access non-public data members.
    friend class boost::serialization::access;
    // Implement serialize method
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & time;
        ar & data;
        
    } 
    int idx;
  
public:
    std::array<std::array<T,N>,MAX_REC_LEN> getData(){
        return this->data;
    }
    std::array<unsigned long,MAX_REC_LEN> getTime(){
        return this->time;
    }
    int getLen(){
        return idx;
    }
    bool PushData(unsigned long curTime,std::array<T,N> inData){
        //return false if it is full (current data still get in, just no more)
        data[idx] = inData;
        time[idx]=curTime;
        idx++;
        if(idx==MAX_REC_LEN){
            idx = 0;
            return false;
        }
        return true;
    }
    RecData(){
        idx = 0;
    }
    ~RecData(){ 
    }

    void PrintData(std::ostringstream &vts,int len){
        for(int i=0;i<len;i++){
            vts<<std::to_string(time[i])<<',';
            std::array<T,N> row = data[i];
            std::copy(row.begin(),row.end()-1,std::ostream_iterator<T>(vts,","));
            vts<<row.back()<<'\n';
        }
    }
    
};

#endif //RECDATA_HPP
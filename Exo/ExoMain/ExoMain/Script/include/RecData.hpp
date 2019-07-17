#ifndef RECDATA_HPP
#define RECDATA_HPP
#include <vector>

template<class T>
class RecData
{
private:
    std::vector<std::vector<T>> data;
    std::vector<unsigned long> time;
    // Allow serialization to access non-public data members.
    friend class boost::serialization::access;
    // Implement serialize method
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & data;
        ar & time;
    } 
  
  
public:
    std::vector<std::vector<T>> getData(){
        return this->data;
    }
    std::vector<unsigned long> getTime(){
        return this->time;
    }
    void PushData(std::vector<T> inData);
    void PushTime(unsigned long curTime);
};
template<class T>
void RecData<T>::PushData(std::vector<T> inData){
    this->data.push_back(inData);
}
template<class T>
void RecData<T>::PushTime(unsigned long curTime){
    this->time.push_back(curTime);
}

#endif //RECDATA_HPP
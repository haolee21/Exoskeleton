#ifndef MOVAVGFILT_HPP
#define MOVAVGFILT_HPP
#include <cmath>
template<int N>
class MovAvgFilt
{
private:
    int SenData[N];
    int Sum;
    int idx;
    unsigned int divider;
    unsigned int findDivider();
    void Add_roundIdx();

public:
    MovAvgFilt(/* args */);
    ~MovAvgFilt();
    int DataFilt(int mea);
};
template<int N>
MovAvgFilt<N>::MovAvgFilt(/* args */)
{
    for (int i = 0; i < N;i++){
        this->SenData[i] = 0;
    }
    this->divider = this->findDivider();
    
}
template<int N>
MovAvgFilt<N>::~MovAvgFilt()
{
}
template<int N>
unsigned int MovAvgFilt<N>::findDivider(){
    return (unsigned int)std::floor(std::log(N) / std::log(2) + 0.5);
}
template<int N>
int MovAvgFilt<N>::DataFilt(int mea){
    this->Sum -= this->SenData[idx];
    this->Sum += mea;
    this->SenData[idx] = mea;
    this->Add_roundIdx();
    return (Sum >> this->divider);
}
template<int N>
void MovAvgFilt<N>::Add_roundIdx(){
    this->idx++;
    if(this->idx==N){
        this->idx = 0;
    }
}
#endif
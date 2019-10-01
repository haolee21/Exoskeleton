#ifndef BWFILTER_HPP
#define BWFILTER_HPP
#include "common.hpp"
#include <memory>
using namespace std;

#define FILTER_ORDER 3
class BWFilter
{
private:
    shared_ptr<unsigned int[]> buf1;
    shared_ptr<unsigned int[]> buf2;
    shared_ptr<unsigned int[]> buf3;

    shared_ptr<unsigned int[]> bufList[FILTER_ORDER] = {buf1,buf2,buf3};
    
    shared_ptr<float[]> outBuf1;
    shared_ptr<float[]> outBuf2;
    shared_ptr<float[]> outBuf3;
    shared_ptr<float[]> outBufList[FILTER_ORDER] = {outBuf1,outBuf2,outBuf3};
    
    
    //Lowpass butterworth filter, this can be implented to arduino if we replace arduino mega with better MCU chips
	//The sampling frequency is 625 Hz
	// cut-off freq is 20 Hz
	//y[n] = (b0*x[n]+b1*x[n-1]+b2*x[n-2]+b3*x[n-3]-a1*y[n-1]-a2*y[n-2]-a3*y[n-3])
    const float b3 = 0.0008;
	const float b2 = 0.0025;
	const float b1 = 0.0025;
	const float b0 = 0.0008;

	const float a3 = -0.6684;
	const float a2 = 2.2737;
	const float a1 = -2.5985;
public:
    BWFilter(/* args */);
    ~BWFilter();
    void FilterData();
};



#endif

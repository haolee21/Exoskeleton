#include "Controller.h"
#include <iostream>
#include <wiringPi.h>

typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
void Controller::TestValve()
{

    for (int i = 0; i < this->ValNum; i++)
    {

        std::cout << "Test Valve " << i << std::endl;
        for (int i2 = 0; i2 < 20; i2++)
        {
            this->ValveList[i].On();
            this->Sleep(200);
            this->ValveList[i].Off();
            this->Sleep(200);
        }
    }
}
void Controller::Sleep(int sleepTime)
{
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now(); //starting time
    while (std::chrono::system_clock::now() - startTime < std::chrono::milliseconds(sleepTime))
    {
        this->WaitToSync();
    }
}
void Controller::WaitToSync()
{
    struct timespec ts = {0};
    ts.tv_sec = 0;
    ts.tv_nsec = 10000L;
    nanosleep(&ts, (struct timespec *)NULL);
}

void Controller::Wait(long waitMilli)
{
    struct timespec ts = {0};
    ts.tv_sec = 0;
    ts.tv_nsec = waitMilli*1000000;
    nanosleep(&ts, (struct timespec *)NULL);
}

void Controller::TestSen()
{
    std::cout << "get into con test\n";
    int curTime;
    //std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now(); //starting time
    for (int i = 0; i < 30; i++)
    {

        try
        {
            std::lock_guard<std::mutex> lock(*this->senLock);
            curTime = this->sensor->senData[0];
        }
        catch (std::logic_error &)
        {
            std::cout << "error\n";
        }
        //std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        //microsecs_t t_duration(std::chrono::duration_cast<microsecs_t>(endTime - startTime));
        //cout <<"we stuck for "<< t_duration.count() << " us \n";
        //startTime = endTime;

        std::cout << curTime - this->preTime << std::endl;
        this->preTime = curTime;
        this->Wait(10);
    }
}
void Controller::TestMeasurement()
{
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point nowTime;
    //This function will test how fast can we respond to measurements
    Valve testOut = Valve("TestMea", 8); //this means gpio2
    for (int i = 0; i < 200; i++)
    {
        int curMea = 300;
        testOut.Off();
        startTime = std::chrono::system_clock::now();
        while (curMea > 60)
        {
            try
            {
                //std::lock_guard<std::mutex> lock(*this->senLock);
                curMea = this->sensor->senData[9];
            }
            catch (std::logic_error &)
            {
                std::cout << "error\n";
            }
            this->Wait(1);
        }
        nowTime = std::chrono::system_clock::now();
        millisecs_t t_duration(std::chrono::duration_cast<millisecs_t>(nowTime - startTime));
        std::cout << "Delay " << t_duration.count() << std::endl;
        this->Wait(1000);
        testOut.On();
        startTime = std::chrono::system_clock::now();
        while (curMea < 200)
        {
            try
            {
                //std::lock_guard<std::mutex> lock(*this->senLock);
                curMea = this->sensor->senData[9];
            }
            catch (std::logic_error &)
            {
                std::cout << "error\n";
            }
            this->Wait(1);
        }
        nowTime = std::chrono::system_clock::now();
        millisecs_t t_duration2(std::chrono::duration_cast<millisecs_t>(nowTime - startTime));
        std::cout << "Delay " << t_duration2.count() << std::endl;
        
        this->Wait(1000);
    }
}

Controller::Controller(Sensor *sensor, mutex *senLock)
{
    this->sensor = sensor;
    this->senLock = senLock;
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();

    this->preTime = 0;
    //turn off all the valves and set start time
    for (int i = 0; i < this->ValNum; i++)
    {
        this->ValveList[i].SetStartTime(startTime);
        this->ValveList[i].Off();
    }
}

Controller::~Controller()
{
}

#include "Sensor.h"
#include <memory>
#include <future>
#define SENSOR_PRIORITY (99)		 /* we use 49 as the PRREMPT_RT use 50 \
									as the priority of kernel tasklets     \
									and interrupt handler by default */
#define POOLSIZE (200 * 1024 * 1024) // 200MB
#define MAX_SAFE_STACK (100 * 1024)	 /* The maximum stack size which is \
									  guranteed safe to access without  \
									  faulting */

typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<unsigned long, std::micro> microsecs_ul;
typedef std::chrono::duration<int, std::milli> millisecs_t;
Sensor::Sensor(std::string _filePath, char *portName, long sampT, std::shared_ptr<Com> _com, bool _display)
{

	//define the encoders
	this->LHip_s.reset(new Encoder(0,0));
	this->LHip_f.reset(new Encoder(1,0));
	this->LKne_s.reset(new Encoder(2,0));
	this->LAnk_s.reset(new Encoder(3,0));
	
	this->RHip_s.reset(new Encoder(0,5));
	this->RHip_f.reset(new Encoder(1,5));
	this->RKne_s.reset(new Encoder(2,5));
	this->RAnk_s.reset(new Encoder(3,5));

	this->adc1.reset(new ADC("/dev/spi1.0"));
	this->adc2.reset(new ADC("/dev/spi1.1"));

	this->display = _display;

	this->filePath = _filePath;
	std::cout << "Create Sensor" << endl;

	this->sampT = sampT * USEC;



	this->senDataLock = new std::mutex;


	

	this->senRec.reset(new Recorder<int,16>("sen", _filePath, "Time,LHipPos,LKnePos,LAnkPos,RHipPos,RKnePos,RAnkPos,sen7,sen8,TankPre,LKnePre,LAnkPre,RKnePre,RAnkPre,sen14,sen15,sen16"));
	//Controller
	this->com = _com.get();

	this->SampTimeRec.reset(new Recorder<int,1>("sampTime", _filePath, "duration"));
}


void Sensor::Start(std::chrono::system_clock::time_point startTime)
{

	this->origin = startTime;
	this->sw_senUpdate = true;

	//initialize the butterworth filter

	//this->th_SenUpdate.reset(new thread(&Sensor::senUpdate, this));

	struct sched_param param;
	Common::initialize_memory_allocation();
	param.sched_priority = SENSOR_PRIORITY;

	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
	{
		perror("sched_setscheduler failed");
		exit(-1);
	}
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
	{
		perror("mlockall failed");
		exit(-2);
	}

	/* Pre-fault our stack */
	Common::stack_prefault();

	// if(pthread_attr_init(&this->attr))
	// 	exo_error(1);
	// if(pthread_attr_setstacksize(&this->attr,PTHREAD_STACK_MIN+MY_STACK_SIZE))
	// 	exo_error(2);
	pthread_attr_init(&this->attr);
	pthread_attr_setstacksize(&this->attr, PTHREAD_STACK_MIN + MY_STACK_SIZE);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&this->th_SenUpdate, &this->attr, &Sensor::senUpdate, this);
	std::cout << "initial receiving thread" << endl;
}
void Sensor::Stop()
{

	std::cout << "get into stop" << endl;
	{
		std::lock_guard<mutex> lock(this->senUpdateLock);
		this->sw_senUpdate = false;
	}
	std::cout << "sen stop flag on\n";

	pthread_join(this->th_SenUpdate, NULL);
	std::cout << "sen loop joined\n";

	std::cout << "sensor fully stops\n";
}
//timer
// void Sensor::tsnorm(struct timespec *ts)
// {
//     while (ts->tv_nsec >= NSEC_PER_SEC)
//     {
//         ts->tv_nsec -= NSEC_PER_SEC;
//         ts->tv_sec++;
//     }
// }
//
void *Sensor::senUpdate(void *_sen)
{
	Sensor *sen = (Sensor *)_sen;

	//for accurate timer

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	
	bool curSenCond = true;
	while (true)
	{
		//I am not sure if multiple ioctl can take place at the same time, we will give it a try 
		future<int> lHipPos_s = std::async(launch::async,[sen]{return sen->LHip_s->ReadPos();});
		future<int> lHipPos_f = std::async(launch::async,[sen]{return sen->LHip_f->ReadPos();});
		future<int> lKnePos_s = std::async(launch::async,[sen]{return sen->LKne_s->ReadPos();});
		future<int> lAnkPos_s = std::async(launch::async,[sen]{return sen->LAnk_s->ReadPos();});
		

		future<int> rHipPos_s = std::async(launch::async,[sen]{return sen->RHip_s->ReadPos();});
		future<int> rHipPos_f = std::async(launch::async,[sen]{return sen->RHip_f->ReadPos();});
		future<int> rKnePos_s = std::async(launch::async,[sen]{return sen->RKne_s->ReadPos();});
		future<int> rAnkPos_s = std::async(launch::async,[sen]{return sen->RAnk_s->ReadPos();});
		//while both adc access the same adcData, since it is addressing different position, we don't need a lock here
		future<void> adc1_data = std::async(launch::async,[sen]{sen->adc1->Read(sen->adcData.data());});
		future<void> adc2_data = std::async(launch::async,[sen]{sen->adc2->Read(sen->adcData.data()+8);});
		lHipPos_s.wait();
		lHipPos_f.wait();
		lKnePos_s.wait();
		lAnkPos_s.wait();
		rHipPos_s.wait();
		rHipPos_f.wait();
		rKnePos_s.wait();
		rAnkPos_s.wait();
		adc1_data.wait();
		adc2_data.wait();
		{
			std::lock_guard<std::mutex> lock(sen->senUpdateLock);
			auto elapsed = std::chrono::high_resolution_clock::now() - sen->origin;
			sen->time = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
			sen->encData[LHIPPOS_S] = lHipPos_s.get();
			sen->encData[LHIPPOS_F]=lHipPos_f.get();
			sen->encData[LKNEPOS_S]=lKnePos_s.get();
			sen->encData[LANKPOS_S]=lAnkPos_s.get();
			sen->encData[RHIPPOS_S]=rHipPos_s.get();
			sen->encData[RHIPPOS_F]=rHipPos_f.get();
			sen->encData[RKNEPOS_S]=rKnePos_s.get();
			sen->encData[RANKPOS_S]=rAnkPos_s.get();
		}
		
		curSenCond = sen->sw_senUpdate;
		
		if (!curSenCond)
		{
			break;
		}

		// timer
		// calculate next shot

		t.tv_nsec += sen->sampT;
		Common::tsnorm(&t);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		clock_gettime(CLOCK_MONOTONIC, &t);
	}
	std::cout << "end sampling\n";
	std::cout << "sensor ends" << endl;
	sen->saveData_th.reset(new std::thread(&Sensor::SaveAllData, sen)); //original purpose is for some reason, sen is destoried before it went through this line
	sen->saveData_th->join();
	//sen->senRec.reset();

	return 0;
}



void Sensor::SaveAllData()
{
	this->senRec.reset();
}

Sensor::~Sensor()
{
	// if(this->saveData_th)
	// 	this->saveData_th->join();
	std::cout << "start to delete" << std::endl;

	// delete this->senDataLock;// I probably cannot delete a mutex element, not sure why
}

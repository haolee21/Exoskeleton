#include "Sensor.h"
#include <memory>

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
	this->LHip_s.reset(new Encoder(0));
	this->LHip_f.reset(new Encoder(1));

	this->display = _display;

	this->filePath = _filePath;
	std::cout << "Create Sensor" << endl;

	this->sampT = sampT * USEC;

	//initialize data receiving buffer
	this->senBuff = SenBuffer();

	this->senDataLock = new std::mutex;
	//this->senDataLock.reset(new std::mutex());

	this->init = false;
	this->falseSenCount = 0;

	this->senData.reset(new int[NUMSEN + 1]);
	this->oriData.reset(new int[NUMSEN]);
	this->senDataRaw.reset(new char[DATALEN]);

	memset(this->senData.get(), 0, NUMSEN + 1);
	memset(this->serialBuf, '\0', SIZEOFBUFFER);
	memset(this->senDataRaw.get(), '\0', DATALEN);
	

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
	std::unique_ptr<Controller> con;
	con.reset(new Controller(sen->filePath, sen->com, sen->display, sen->origin, sen->sampT));
	std::unique_ptr<std::thread> conTh;
	bool conStart = false;

	//for accurate timer

	// struct timespec t;
	// clock_gettime(CLOCK_MONOTONIC, &t);

	// int flushVal = tcflush(sen->serialDevId, TCIFLUSH);
	// if(flushVal==0){
	// 	std::cout << "clear buffer\n";
	// }
	// else{
	// 	std::cout << "failed to clear the buffer\n";
	// }
	// sen->ResetPin->Off();
	bool curSenCond = true;
	while (true)
	{

		if (conStart)
		{
			// if(conTh->joinable()){ //I am not sure but the program stop to freeze after I put this
			// 	conTh->join();
			// }
		}
		else
		{
			conStart = true;
			con->Start(sen->senData.get(), sen->senDataRaw.get(), sen->senDataLock);
		}

		{
			std::lock_guard<std::mutex> lock(sen->senUpdateLock);
			curSenCond = sen->sw_senUpdate;
		}
		if (!curSenCond)
		{
			break;
		}

		// timer
		// calculate next shot

		// t.tv_nsec += sen->sampT;
		// Common::tsnorm(&t);
		// clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		//clock_gettime(CLOCK_MONOTONIC, &t);
	}
	std::cout << "end sampling\n";
	con->Stop();
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

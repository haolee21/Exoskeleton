import multiprocessing as mp
import threading as th
import Valve 
import time
import gpiozero as gp

## Position measurement        
LHIPPOS = 0        
LKNEPOS = 5        
LANKPOS = 6        
RHIPPOS = 0       
RKNEPOS = 1        
RANKPOS = 2        
LKNEVEL = 17        
## Pressure measurement index        
LKNEPRE = 16        
LTANKPRE = 1        
RKNEPRE = 3        
RPRETANk = 4        
LANKPRE = 8        
RTANKPRE = 14



OP1 = 14
OP2 = 15        
OP3 = 18        
OP4 = 23        
OP5 = 24        
OP6 = 25        
OP7 = 8        
OP8 = 12        
OP9 = 16        
OP10 = 20        
OP11 = 21        
OP12 = 2        
OP13 = 3        
OP14 = 17        
OP15 = 27        
OP16 = 22

swValL = Valve.Valve('swValL',OP6)
preVal1 = Valve.Valve('preVal1',OP1)
tankVal1 = Valve.Valve('tankVal1',OP5)
preVal2 = Valve.Valve('preVal2',OP2)
actVal = Valve.Valve('actVal',OP8)
tankVal2 = Valve.Valve('tankVal2',OP7)
relPreVal = Valve.Valve('relPreVal',OP3)
balVal = Valve.Valve('balVal',OP4)
addVal1 = Valve.Valve('addVal1',OP9)
addVal2 = Valve.Valve('addVal2',OP10)
addVal3 = Valve.Valve('addVal3',OP11)
addVal4 = Valve.Valve('addVal4',OP12)
addVal5 = Valve.Valve('addVal5',OP13)
addVal6 = Valve.Valve('addVal6',OP14)
addVal7 = Valve.Valve('addVal7',OP15)
addVal8 = Valve.Valve('addVal8',OP16)

syncPin = gp.OutputDevice(4)

class ValveController(object):

    j_recLHip = th.Event()
    j_recLKne = th.Event()
    j_recRHip = th.Event()
    j_recRKne = th.Event()
    j_testLowFreqValve = th.Event()
    j_testHighFreqValve = th.Event()
    j_testAllValve = th.Event()
    j_testAct = th.Event()
    j_balance = th.Event()
    j_testSync = th.Event()
    j_recLSpring = th.Event()
    j_testBreak = th.Event()
    """description of class"""
    def __init__(self,conFreq,cmdFreq,cmdQue,cmdLock,senArray):
        self.conFreq = conFreq # the frequency of control loop,
                               # should be some value less than the loop itself,
                               # since it is just for avoiding control loop run too fast that starve the senArray
        self.cmdFreq = cmdFreq # the frequency of refreshing command
        self.cmdQue = cmdQue
        self.cmdLock = cmdLock
        self.senArray = senArray
        self.switch=mp.Event()

        self.pwmVal1 = Valve.Valve('PWM1',0)
        self.pwmVal2 = Valve.Valve('PWM2',1)
    def start(self):
        # When start, first sync the time 
        self.switch.set()
        self.syncConAndSen()
        checkTaskTh = th.Thread(target=self.checkTasks,args=(self.cmdQue,self.cmdLock,))
        checkTaskTh.start()
        conLoopTh = th.Thread(target=self.conLoop)
        conLoopTh.start()
    def stop(self):
        self.switch.clear()
    def selfSync(self,syncTime):
        time.sleep(syncTime)
    def syncConAndSen(self):
        timeSync = []
        timeDiff=[0.05,0.1,0.3]
        for t_sleep in timeDiff:
            syncPin.on()
            timeSync.append(time.time())
            time.sleep(t_sleep)
            syncPin.off()
            timeSync.append(time.time())
            time.sleep(t_sleep)

        print('Done sycn')
    def noTask(self):
        print('No such task')

    def checkTasks(self,cmdQue,cmdQueLock):

        while self.switch.is_set():
            selfSync = th.Thread(target=self.selfSync,args=(1/self.cmdFreq,))
            selfSync.start()
            # now we check the command
            if not cmdQue.empty():
                with cmdQueLock:
                    curCmd = cmdQue.get()
                if curCmd[0]=='reclkne':
                    if curCmd[1]=='start':
                        self.j_recLKne.set()
                    elif curCmd[1]=='stop':
                        self.j_recLKne.clear()
                    else:
                        self.noTask()
                elif curCmd[0]=='recspring':
                    if curCmd[1]=='start':
                        self.j_recLSpring.set()
                    elif curCmd[1]=='stop':
                        self.j_recLSpring.clear()
                    else:
                        self.noTask()

            selfSync.join()


    def conLoop(self):
        while self.switch.is_set():
            #print('control loop')
            selfSync = th.Thread(target=self.selfSync(1/self.conFreq,))
            selfSync.start()
            allTaskList =[]
            # Get current measurement
            curSen = list(self.senArray) #todo make sure this will not cause trouble if we read and write at the same time
            allTaskList.append(th.Thread(target=self.testTask))
            if self.j_recLSpring.is_set():
                allTaskList.append(th.Thread(targe=self.recLSpring))




            for task in allTaskList:
                task.start()
            for task in allTaskList:
                task.join()

            selfSync.join()

    def recLSpring(self):
        test = 1
        #print('do job')
    def testTask(self):
        #print('try')
        time.sleep(2)
        #print('done try')




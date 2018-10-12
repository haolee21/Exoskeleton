import multiprocessing as mp
import threading as th
import Valve 
import time
import gpiozero as gp
import PWMGen as pwm
## Position measurement
TIME = 0
LHIPPOS = 1
LKNEPOS = 2
LANKPOS = 3




RHIPPOS = 0       
RKNEPOS = 1        
RANKPOS = 2        
LKNEVEL = 17        
## Pressure measurement index        
LKNEPRE = 6
LTANKPRE = 5
RKNEPRE = 3        
RPRETANk = 4        
LANKPRE = 7
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

# swValL = Valve.Valve('swValL',OP6)
# preVal1 = Valve.Valve('preVal1',OP1)
# tankVal1 = Valve.Valve('tankVal1',OP5)
# preVal2 = Valve.Valve('preVal2',OP2)
# actVal = Valve.Valve('actVal',OP8)
# tankVal2 = Valve.Valve('tankVal2',OP7)
# relPreVal = Valve.Valve('relPreVal',OP3)
# balVal = Valve.Valve('balVal',OP4)
# addVal1 = Valve.Valve('addVal1',OP9)
# addVal2 = Valve.Valve('addVal2',OP10)
# addVal3 = Valve.Valve('addVal3',OP11)
# addVal4 = Valve.Valve('addVal4',OP12)
#
# kneVal1 = Valve.Valve('kneVal1',OP13)
# kneVal2 = Valve.Valve('kneVal2',OP14)
# ankVal1 = Valve.Valve('ankVal1',OP15)
# ankVal2 = Valve.Valve('ankVal2',OP16)



syncPin = gp.OutputDevice(4)


# Threshold for finite state machine
    # Encoder parameters
    # ankThMid: 3.8716 kOhm/5.08 kOhms
    # kneThHigh: 4.16 kOhm/4.4351 kOhms
    # kneThLow: 3.542 kOhm/4.4351 kOhms
    # hipThMid: 2.9 kOhm/5.049 kOhms
    # hipThHigh: 3.05 kOhm/5.049 kOhms
    # Vcc = 4.97 Volts
    # arduino measurement: 5 volt/1024 units

ankThMid = 3.8716/5.08*4.97*1024/5   # 775.7345058267715
kneThHigh=4.16/4.4351*4.97*1024/5    # 954.7205158846475
kneThLow=3.542/4.4351*4.97*1024/5    # 812.8894392460146
hipThMid=2.9/5.049*4.97*1024/5       # 584.6271340859576
hipThHigh = 3.05/5.049*4.97*1024/5   # 614.866468607645


class ValveController(object):
    j_recL = th.Event()
    j_recLHip = th.Event()
    j_recLKne = th.Event()
    j_recRHip = th.Event()
    j_recRKne = th.Event()
    j_testLowFreqValve = th.Event()
    j_testHighFreqValve = th.Event()
    j_testAllValve = th.Event()
    j_testAct = th.Event()
    j_balance = th.Event()

    j_recLSpring = th.Event()
    j_testBreak = th.Event()
    j_test = th.Event()
    j_valveTest = th.Event()
    j_testSync = th.Event()
    j_testPWM = th.Event()
    """description of class"""
    def __init__(self,conFreq,cmdFreq,cmdQue,cmdLock,senArray,valveRecQue,valveRecLock,syncTimeQue,pwmRecQue,pwmRecLock):
        print('#start init valve controller')
        self.conFreq = conFreq # the frequency of control loop,
                               # should be some value less than the loop itself,
                               # since it is just for avoiding control loop run too fast that starve the senArray
        self.cmdFreq = cmdFreq # the frequency of refreshing command
        self.cmdQue = cmdQue
        self.cmdLock = cmdLock
        self.senArray = senArray
        self.switch=mp.Event()
        self.syncTimeQue = syncTimeQue

        # initialize finite state machine
        self.state = 1

        # define valve (need to record)

        self.kneVal1 = Valve.Valve('KneVal1',OP9,valveRecQue,valveRecLock)
        self.kneVal2 = Valve.Valve('KneVal2',OP4,valveRecQue,valveRecLock)
        self.ankVal1 = Valve.Valve('AnkVal1',OP6,valveRecQue,valveRecLock)
        self.ankVal2 = Valve.Valve('AnkVal2',OP7,valveRecQue,valveRecLock)
        #self.knePreVal = Valve.Valve('KnePreVal',OP1,valveRecQue,valveRecLock)
        #self.ankPreVal = Valve.Valve('AnkPreVal',OP2,valveRecQue,valveRecLock)
        self.balVal = Valve.Valve('BalVal',OP5,valveRecQue,valveRecLock)

        self.valveList = [self.kneVal1,self.kneVal2,self.ankVal1,self.ankVal2,self.balVal]

        # define pwm valves
        self.pwmRecQue = pwmRecQue
        self.pwmRecLock = pwmRecLock
        self.knePreValPWM = pwm.PWMGen('KnePreVal',OP1,self.pwmRecQue,self.pwmRecLock)
        self.ankPreValPWM = pwm.PWMGen('AnkPreVal',OP2,self.pwmRecQue,pwmRecLock)
        self.pwmValList = [self.knePreValPWM,self.ankPreValPWM]
        print('#done init valve controller')
    def start(self):
        # When start, first sync the time 
        self.switch.set()
        self.syncConAndSen()
        checkTaskTh = th.Thread(target=self.checkTasks,args=(self.cmdQue,self.cmdLock,))
        checkTaskTh.start()
        conLoopTh = th.Thread(target=self.conLoop)
        conLoopTh.start()
        print('#valve controller starts')
    def stop(self):
        self.switch.clear()

    def syncConAndSen(self):
        timeDiff=[0.05,0.1,0.3]
        for t_sleep in timeDiff:
            syncPin.on()
            self.syncTimeQue.put(time.time())
            time.sleep(t_sleep)
            syncPin.off()
            self.syncTimeQue.put(time.time())
            time.sleep(t_sleep)

        print('#Done sycn')
    def noTask(self):
        print('#No such task')
    def selfSync(self,sleepTime):
        time.sleep(sleepTime)

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
                elif curCmd[0]=='rec':
                    if curCmd[1]=='start':
                        self.j_recL.set()
                    elif curCmd[1]=='stop':
                        self.j_recL.clear()
                    else:
                        self.noTask()
                elif curCmd[0]=='test':
                    if curCmd[1]=='start':
                        self.j_test.set()
                    elif curCmd[1]=='stop':
                        self.j_test.clear()
                elif curCmd[0]=='testval':
                    self.j_valveTest.set()
                elif curCmd[0]=='testsync':
                    self.j_testSync.set()
                elif curCmd[0]=='testpwm':
                    self.j_testPWM.set()
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
            #　list(array) will only copy the value
            if self.j_recLSpring.is_set():
                allTaskList.append(th.Thread(target=self.recLSpring,args=(curSen,)))
            if self.j_test.is_set():
                allTaskList.append(th.Thread(target=self.testTask))
            if self.j_valveTest.is_set():
                allTaskList.append(th.Thread(target=self.testValve))
                self.j_valveTest.clear()
            if self.j_testSync.is_set():
                allTaskList.append(th.Thread(target=self.syncConAndSen))
                self.j_testSync.clear()
            if self.j_testPWM.is_set():
                allTaskList.append(th.Thread(target=self.testPwm))
                self.j_testPWM.clear()

            for task in allTaskList:
                task.start()
            for task in allTaskList:
                task.join()

            selfSync.join()

    def recLSpring(self,curSen):
        def phase1():
            self.kneVal1.Off()
            self.kneVal2.Off()
            self.ankVal1.Off()
            self.ankVal2.Off()
            if curSen[LKNEPOS]<kneThLow:
                print('#Phase 2')
                return 2
            else:

                return 1
        def phase2():
            self.kneVal1.On()
            self.kneVal2.On()
            if curSen[LANKPOS]<ankThMid:
                print('#Phase 3')
                return 3
            else:
                return 2
        def phase3():
            self.kneVal1.Off()
            self.kneVal2.Off()
            self.ankVal1.On()
            self.ankVal2.On()
            if curSen[LHIPPOS]>hipThMid:
                print('#Phase 4')
                return 4
            else:
                return 3
        def phase4():
            self.kneVal1.On()
            self.kneVal2.On()
            self.ankVal1.On()
            self.ankVal2.On()
            if curSen[LHIPPOS]>hipThHigh:
                print('#Phase 5')
                return 5
            else:
                return 4
        def phase5():
            self.kneVal1.Off()
            self.kneVal2.Off()
            self.ankVal1.On()
            self.ankVal2.On()
            if curSen[LKNEPOS]>kneThHigh:
                print('#Phase 6')
                return 6
            else:
                return 5
        def phase6():
            self.kneVal1.On()
            self.kneVal2.On()
            self.ankVal1.Off()
            self.ankVal2.Off()
            if curSen[LKNEPOS]<kneThHigh:
                print('#Phase 1')
                return 1
            else:
                return 6
        # use finite machine to control
        # total 7 different phases
        #print(curSen[LHIPPOS],curSen[LKNEPOS],curSen[LANKPOS])
        if self.state ==1:
            self.state = phase1()
        elif self.state==2:
            self.state = phase2()
        elif self.state==3:
            self.state = phase3()
        elif self.state==4:
            self.state = phase4()
        elif self.state==5:
            self.state = phase5()
        elif self.state==6:
            self.state = phase6()
        else:
            raise Exception('Finite state machine cannot classified state')


    def testTask(self):
        print('try')
        time.sleep(0.05)
        print('done try')
        time.sleep(0.05)


    def testValve(self):
        for valve in self.valveList:
            print(valve.name)
            for i in range(10):
                valve.On()
                time.sleep(0.1)
                valve.Off()
                time.sleep(0.1)
        print('#Done testval')

    def testPwm(self):
        waitTime = 5
        dutyTest = [10,50,80]
        print('test knee pwm')
        self.knePreValPWM.start()
        for duty in dutyTest:
            self.knePreValPWM.setDuty(duty)
            print('current duty '+ str(duty))
            time.sleep(waitTime)
        self.knePreValPWM.setDuty(0)
        self.knePreValPWM.stop()

        print('test ankle pwm')
        self.ankPreValPWM.start()
        for duty in dutyTest:
            self.ankPreValPWM.setDuty(duty)
            print('current duty ' + str(duty))
            time.sleep(waitTime)
        self.ankPreValPWM.setDuty(0)
        self.ankPreValPWM.stop()
        print('done pwm test')
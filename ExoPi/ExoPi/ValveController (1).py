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
LTANKPRE = 5
LKNEPRE = 6
LANKPRE = 7

RKNEPRE = 3        
RPRETANk = 4        

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




class ValveController(object):
    j_recL = th.Event()
    j_initSup = th.Event()
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
    j_testAnkAct = th.Event()
    j_testKneAct = th.Event()
    j_recPos = th.Event()
    j_dualPwm = th.Event()
    j_freeWalk = th.Event()
    """description of class"""
    def __init__(self,conFreq,cmdFreq,cmdQue,cmdLock,senArray,senLock,valveRecQue,valveRecLock,syncTimeQue,pwmRecQue1,pwmRecQue2,stateQue):
        print('#start init valve controller')
        self.conT = 1/conFreq # the frequency of control loop,
                               # should be some value less than the loop itself,
                               # since it is just for avoiding control loop run too fast that starve the senArray
        self.cmdFreq = cmdFreq # the frequency of refreshing command
        self.cmdQue = cmdQue
        self.cmdLock = cmdLock
        self.senArray = senArray
        self.senLock=senLock
        self.switch=mp.Event()
        self.syncTimeQue = syncTimeQue

        # initialize finite state machine
        self.state = 1

        # define valve (need to record)

        self.kneVal1 = Valve.Valve('KneVal1',OP9,valveRecQue,valveRecLock)
        self.kneVal2 = Valve.Valve('KneVal2',OP4,valveRecQue,valveRecLock)
        self.ankVal1 = Valve.Valve('AnkVal1',OP6,valveRecQue,valveRecLock)
        self.ankVal2 = Valve.Valve('AnkVal2',OP7,valveRecQue,valveRecLock)
        self.balVal = Valve.Valve('BalVal',OP3,valveRecQue,valveRecLock)
        self.kneRel = Valve.Valve('KneRel',OP8,valveRecQue,valveRecLock)

        self.valveList = [self.kneVal1,self.kneVal2,self.ankVal1,self.ankVal2,self.balVal,self.kneRel]

        # Valve init cond
        self.balVal.on()
        # define pwm valves


        self.knePreValPWM = pwm.PWMGen('KnePreVal',OP1,pwmRecQue1)
        self.ankPreValPWM = pwm.PWMGen('AnkPreVal',OP2,pwmRecQue2)
        self.pwmValList = [self.knePreValPWM,self.ankPreValPWM]


        # record walking state
        self.stateQue = stateQue
        print('#done init valve controller')

        self.change = False
    def start(self):
        # When start, first sync the time 
        self.switch.set()
        checkTaskTh = th.Thread(target=self.checkTasks,args=(self.cmdQue,self.cmdLock,))
        checkTaskTh.start()
        conLoopTh = th.Thread(target=self.conLoop)
        conLoopTh.start()
        print('#valve controller starts')

    def stop(self):
        self.switch.clear()
        time.sleep(5)
        self.ankPreValPWM.stop()
        self.knePreValPWM.stop()
        print('#release pressure')
        time.sleep(5)
        self.kneVal1.off()
        self.kneVal2.off()
        self.kneRel.off()
        self.ankVal1.off()
        self.ankVal2.off()
        self.balVal.off()
        self.knePreValPWM.on()
        self.ankPreValPWM.on()
        time.sleep(10)
        self.knePreValPWM.off()
        self.ankPreValPWM.off()
        print('#done releasing pressure')
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


    def checkTasks(self,cmdQue,cmdQueLock):

        while self.switch.is_set():
            initTime = time.time()

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
                elif curCmd[0]=='recl':
                    if curCmd[1]=='start':
                        self.knePreValPWM.start()
                        self.ankPreValPWM.start() # we start the pwm here since we only start it once
                        self.stateQue.put([time.time()*1000,1])
                        self.j_recL.set()
                    elif curCmd[1]=='stop':
                        self.j_recL.clear()
                        self.knePreValPWM.stop()
                        self.ankPreValPWM.stop()
                    else:
                        self.noTask()
                elif curCmd[0]=='initsup':
                    self.knePreValPWM.start()
                    self.syncConAndSen()
                    self.j_initSup.set()
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
                elif curCmd[0]=='testankact':
                    self.j_testAnkAct.set()
                elif curCmd[0]=='testkneact':
                    self.j_testKneAct.set()
                elif curCmd[0]=='dualpwm':
                    self.j_dualPwm.set()
                elif curCmd[0]=='c':
                    self.change = True
                elif curCmd[0]=='s':
                    self.j_recPos.set()
                elif curCmd[0]=='freewalk':
                    self.syncConAndSen()
                else:
                    self.noTask()
            endTime = time.time()
            while (endTime-initTime)<self.conT:
                time.sleep(0.001)
                endTime = time.time()

    def conLoop(self):
        while self.switch.is_set():
            initTime = time.time()
            #print('control loop')
            allTaskList =[]
            # Get current measurement
            with self.senLock:
                curSen = list(self.senArray) #todo make sure this will not cause trouble if we read and write at the same time
            #　list(array) will only copy the value
            if self.j_recLSpring.is_set():
                allTaskList.append(th.Thread(target=self.recLSpring,args=(curSen,)))
            if self.j_initSup.is_set():
                allTaskList.append(th.Thread(target=self.initSupPre,args=(curSen,)))
            if self.j_recL.is_set():
                allTaskList.append(th.Thread(target=self.recL,args=(curSen,)))
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
            if self.j_testAnkAct.is_set():
                allTaskList.append(th.Thread(target=self.testAnkAct))
                self.j_testAnkAct.clear()
            if self.j_testKneAct.is_set():
                allTaskList.append(th.Thread(target=self.testKneAct))
                self.j_testKneAct.clear()
            if self.j_recPos.is_set():
                self.j_recPos.clear()
                allTaskList.append(th.Thread(target=self.recPos,args=(curSen,)))
            if self.j_dualPwm.is_set():
                self.j_dualPwm.clear()
                allTaskList.append(th.Thread(target=self.testBothPwm))


            for task in allTaskList:
                task.start()
            for task in allTaskList:
                task.join()
            endTime = time.time()
            while (endTime-initTime)<self.conT:
                time.sleep(0.001)
                endTime = time.time()
    def recLSpring(self,curSen):
        pass
    kneSupPre = 300
    # preTh = 10   # remove since the system cannot reach it
    preTh=0
    kneWalkSupPre = 310
    ankWalkActPre = 300

    preDiffTh = 20
    kneThHigh = 500
    kneThLow = 460

    ankThHigh = 580
    ankThLow = 460
    hipThMid = 540
    hipThHigh = 560
    def initSupPre(self,curSen):
        if curSen[LKNEPRE]<(self.kneSupPre-self.preTh):
            # print('!Not enough supporting pressure')
            self.balVal.on()
            self.kneVal1.off()
            self.kneVal2.on()
            dutyKne = self.calDutyAct(curSen[LKNEPRE], curSen[LTANKPRE],self.kneSupPre)
            self.knePreValPWM.setDuty(dutyKne)
        else:
            print('enough supporting pressure')
            self.knePreValPWM.setDuty(0)
            self.knePreValPWM.stop()
            self.balVal.on()
            self.kneVal2.on()
            self.kneVal1.off()
            self.j_initSup.clear()
        # print(curSen[LKNEPRE]-self.kneSupPre)
    def recL(self,curSen):
        def phase1():
            self.balVal.on()
            self.kneVal1.off()
            self.kneVal2.on()
            self.kneRel.on()
            dutyKne = self.calDutyRec(curSen[LKNEPRE],curSen[LTANKPRE])
            self.knePreValPWM.setDuty(dutyKne)
            self.ankVal1.off()
            self.ankVal2.on()
            dutyAnk = self.calDutyRec(curSen[LANKPRE],curSen[LTANKPRE])
            self.ankPreValPWM.setDuty(dutyAnk)

            if curSen[LKNEPOS]<self.kneThLow: #if pressure is not high enough, don't go to phase 2
            #if self.change:
                self.change=False
                print('#Phase 2')
                self.knePreValPWM.setDuty(0)
                self.ankPreValPWM.setDuty(0)
                self.stateQue.put([time.time()*1000,2])
                return 2
            else:
                return 1
        def phase2():

            self.kneVal1.off()
            self.kneVal2.on()
            self.kneRel.on()
            self.ankVal1.on()
            self.ankVal2.on()
            self.balVal.off()
            # dutyAnk = self.calDutyRec(curSen[LANKPRE], curSen[LTANKPRE])
            # self.ankPreValPWM.setDuty(dutyAnk)
            if abs(curSen[LKNEPRE]-curSen[LANKPRE])<self.preDiffTh:
            #if self.change:
                self.change = False
                print('#Phase 3')
                # self.ankPreValPWM.setDuty(0)
                self.stateQue.put([time.time()*1000,3])
                return 3
            else:
                return 2
        def phase3():
            self.kneVal1.on()
            self.kneVal2.on()
            self.kneRel.off()

            self.ankVal1.off()
            self.ankVal2.on()
            self.balVal.on()
            dutyAnk = self.calDutyRec(curSen[LANKPRE], curSen[LTANKPRE])
            self.ankPreValPWM.setDuty(dutyAnk)
            if curSen[LANKPOS]>self.ankThHigh:
            #if self.change:
                self.change=False
                print('#Phase 4')
                self.ankPreValPWM.setDuty(0)
                self.stateQue.put([time.time()*1000, 4])
                return 4
            else:
                return 3
        def phase4():
            self.kneVal1.off()
            self.kneVal2.on()
            self.kneRel.on()
            self.ankVal1.on()
            self.ankVal2.on()
            self.balVal.off()
            if abs(curSen[LKNEPRE] - curSen[LANKPRE]) < self.preDiffTh:
                print('#Phase 5')
                self.stateQue.put([time.time() * 1000, 5])
                return 5
            else:
                return 4
        def phase5():
            self.kneVal1.off()
            self.kneVal2.off()
            self.kneRel.off()

            self.ankVal1.off()
            self.ankVal2.on()
            self.balVal.on()
            ankDuty = self.calDutyAct(curSen[LTANKPRE], curSen[LANKPRE],
                                      self.ankWalkActPre)  # act the ankle with knee pressure

            self.ankPreValPWM.setDuty(ankDuty)
            if curSen[LANKPOS]<self.ankThLow:
            #if self.change:
                self.change=False
                print('#Phase 6')
                self.ankPreValPWM.setDuty(0)
                self.stateQue.put([time.time()*1000, 6])
                return 6
            else:
                return 5
        def phase6():
            self.kneVal1.off()
            self.kneVal2.off()
            self.kneRel.off()

            self.ankVal1.on()
            self.ankVal2.on()
            self.balVal.on()
            if curSen[LHIPPOS]>self.hipThMid:
            #if self.change:
                self.change=False
                print('#Phase 7')
                self.stateQue.put([time.time()*1000, 7])
                return 7
            else:
                return 6
        def phase7():
            self.kneVal1.off()
            self.kneVal2.off()
            self.kneRel.off()

            self.ankVal1.off()
            self.ankVal2.on()
            self.balVal.off()
            if (curSen[LKNEPOS]>self.kneThHigh) and (curSen[LHIPPOS]>self.hipThHigh):
            #if self.change:
                self.change=False
                self.stateQue.put([time.time()*1000, 8])
                print('#Phase 8')
                return 8
            else:
                return 7
        def phase8():
            self.balVal.on()
            self.kneVal1.on()
            self.kneVal2.on()
            self.kneRel.on()
            self.ankVal1.on()
            self.ankVal2.on()


            kneDuty = self.calDutyAct(curSen[LTANKPRE],curSen[LKNEPRE],self.kneWalkSupPre)
            self.knePreValPWM.setDuty(kneDuty)
            if curSen[LKNEPOS]<self.kneThHigh:
            #if self.change:
                self.change=False
                self.stateQue.put([time.time()*1000, 1])
                print('#Phase 1')
                return 1
            else:
                return 8

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
        elif self.state==7:
            self.state = phase7()
        elif self.state==8:
            self.state= phase8()
        else:
            raise Exception('!Finite state machine cannot classified state')

    def calDutyAct(self,driPre,conPre,conDes):
        if (driPre-conPre)==0:
            duty = 90
        else:
            duty = (conDes-conPre)/(driPre-conPre)*50
        if duty<0:
            duty=0
        elif duty>90:
            duty = 90
        return  duty
    def calDutyRec(self,recPre,tankPre):
        diff = recPre -tankPre
        if diff>50:
            duty = 90
        elif diff>30:
            duty = 50
        elif diff>10:
            duty = 20
        else:
            duty =0
        return duty



    def testTask(self):
        print('try')
        time.sleep(0.05)
        print('done try')
        time.sleep(0.05)


    def testValve(self):
        for valve in self.valveList:
            print(valve.name)
            for i in range(10):
                valve.on()
                time.sleep(0.1)
                valve.off()
                time.sleep(0.1)
        print('#Done testval')

    def testPwm(self):
        waitTime = 5
        dutyTest = [10,50,80]
        print('#test knee pwm')
        self.knePreValPWM.start()
        for duty in dutyTest:
            self.knePreValPWM.setDuty(duty)
            print('#current duty '+ str(duty))
            time.sleep(waitTime)
        self.knePreValPWM.setDuty(0)
        self.knePreValPWM.stop()

        print('#test ankle pwm')
        self.ankPreValPWM.start()
        for duty in dutyTest:
            self.ankPreValPWM.setDuty(duty)
            print('#current duty ' + str(duty))
            time.sleep(waitTime)
        self.ankPreValPWM.setDuty(0)
        self.ankPreValPWM.stop()
        print('#done pwm test')
    def testAnkAct(self):
        self.balVal.on()
        self.ankPreValPWM.setDuty(90)
        self.ankPreValPWM.start()
        self.ankVal2.on()
        self.ankVal1.off()
        time.sleep(10)
        self.ankPreValPWM.setDuty(0)
        self.ankPreValPWM.stop()
        self.ankVal2.off()
        self.balVal.off()
        time.sleep(10)
        print('#done ankle act test')
    def testKneAct(self):
        self.balVal.on()
        self.knePreValPWM.setDuty(90)
        self.knePreValPWM.start()
        self.kneVal2.on()
        self.kneVal1.off()
        time.sleep(10)
        self.knePreValPWM.setDuty(0)
        self.knePreValPWM.stop()
        self.kneVal2.off()
        self.balVal.off()
        time.sleep(10)
        print('#done knee act test')
    def testBothPwm(self):
        waitTime = 5
        dutyTest = [10, 50, 80]

        print('#test both pwm')
        self.knePreValPWM.start()
        self.ankPreValPWM.start()
        for duty in dutyTest:
            self.knePreValPWM.setDuty(duty)
            self.ankPreValPWM.setDuty(100-duty)
            print('#current knee  duty:'+str(duty))
            print('#        ankle duty:'+str(100-duty))
            time.sleep(waitTime)
        self.knePreValPWM.setDuty(0)
        self.ankPreValPWM.setDuty(0)
        self.knePreValPWM.stop()
        self.ankPreValPWM.stop()
        print('done dual pwm test')
    def recPos(self,curSen):
        print('\nHip\t'+str(curSen[LHIPPOS]))
        print('Knee\t'+str(curSen[LKNEPOS]))
        print('Ankle\t'+str(curSen[LANKPOS]))


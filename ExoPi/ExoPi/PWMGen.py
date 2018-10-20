import multiprocessing as mp
import time
import Valve
import threading as th
import gpiozero as gp
pwmUnit = 300
class PWMGen(object):
    """description of class"""
    def __init__(self,name,index,pwmRecQue):
        self.name = name
        self.pwmRecQue = pwmRecQue

        self.pwmVal = gp.OutputDevice(index)
        self.switch = mp.Event()
        self.dutyQue = mp.Queue(1)
        self.dutyLock = th.Lock()
        self.dutyQue.put(0)
        self.setDuty(0)
        

    def start(self):
        print('!start\t'+str(self.name))
        if not self.switch.is_set():
            self.switch.set()
        mainThread = th.Thread(target=self.main)
        if self.dutyQue.empty():
            print('!No input duty for PWM Generator\t'+str(self.name))
        else:
            mainThread.start()
    def stop(self):
        if self.switch.is_set():
            self.switch.clear()

    def setDuty(self,duty):
        with self.dutyLock:
            curTime = time.time()
            self.dutyQue.get()
            self.dutyQue.put(duty)
        thread = th.Thread(target=self.recDuty,args=(curTime,duty,))
        thread.start()
    def main(self):
        while self.switch.is_set():
            initTime = time.time()
            with self.dutyLock:
                curDuty = self.dutyQue.get()
                self.dutyQue.put(curDuty)
            onTime = curDuty/float(100)*pwmUnit/6000

            if onTime<0.00005:
                self.pwmVal.off()
                time.sleep(pwmUnit/6000)
            else:
                self.pwmVal.on()
                time.sleep(onTime)
            self.pwmVal.off()
            endTime = time.time()

            while (endTime-initTime)<(pwmUnit/6000):
                time.sleep(0.0005)
                endTime = time.time()
        print('!'+str(self.name)+'\t stops')
    def recDuty(self,curTime,curDuty):
        self.pwmRecQue.put(str(curTime)+','+self.name+','+str(curDuty))
    def on(self):
        self.pwmVal.on()
    def off(self):
        self.pwmVal.off()
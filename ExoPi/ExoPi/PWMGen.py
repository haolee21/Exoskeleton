import multiprocessing as mp
import time
import Valve
import threading as th
import gpiozero as gp
pwmUnit = 300
class PWMGen(object):
    """description of class"""
    def __init__(self,name,index,pwmRecQue,pwmRecLock):
        self.name = name
        self.pwmRecQue = pwmRecQue
        self.pwmRecLock = pwmRecLock
        self.pwmVal = gp.OutputDevice(index)
        self.switch = mp.Event()
        self.dutyQue = mp.Queue()
        self.dutyLock = th.Lock()
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
        self.switch.clear()

    def setDuty(self,duty):
        curTime = time.time()
        with self.dutyLock:
            self.dutyQue.put(duty)
        thread = th.Thread(target=self.recDuty,args=(curTime,duty,))
        thread.start()
    def main(self):
        while self.switch.is_set():
            if not self.dutyQue.empty():
                curDuty = self.dutyQue.get()
            onTime = curDuty/float(100)*pwmUnit/6000
            offTime = (100 - curDuty) / float(100) * pwmUnit/6000
            if onTime<0.00005:
                self.pwmVal.off()
                time.sleep(pwmUnit/6000)
                continue
            self.pwmVal.on()
            time.sleep(onTime)
            self.pwmVal.off()
            time.sleep(offTime)
        print('!'+str(self.name)+'\t stops')
    def recDuty(self,curTime,curDuty):
        with self.pwmRecLock:
            self.pwmRecQue.put(str(curTime)+','+self.name+','+str(curDuty))
    def on(self):
        self.pwmVal.on()
    def off(self):
        self.pwmVal.off()
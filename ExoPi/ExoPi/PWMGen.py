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
        self.dutyQue.put(0) #give initial value
        self.dutyLock = th.Lock()
        self.mainThread = th.Thread(target = self.main)

    def start(self):
        self.switch.set()
        if self.dutyQue.is_empty():
            print('!No input duty for PWM Generator')
        else:
            self.mainProcess.start()
    def stop(self):
        self.switch.clear()
    def setDuty(self,duty):
        with self.dutyLock:
            self.dutyQue.put(duty)

    def main(self):
        while self.swtich.is_set():
            if not self.dutyQue.empty():
                curDuty = self.dutyQue.get()
            onTime = curDuty/float(100)*pwmUnit/6000
            offTime = (100 - curDuty) / float(100) * pwmUnit/6000
            if onTime<0.005:
                self.pwmVal.Off()
                time.sleep(pwmUnit/6000)
                continue
            self.pwmVal.On()
            time.sleep(onTime)
            self.pwmVal.Off()
            time.sleep(offTime)
            
    def recDuty(self,curTime,curDuty):
        with self.pwmRecLock:
            self.pwmRecQue.put(str(curTime)+','+self.name+','+curDuty)
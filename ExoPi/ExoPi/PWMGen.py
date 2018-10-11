import multiprocessing as mp
import time
import Valve
import threading as th
pwmUnit = 300
class PWMGen(object):
    """description of class"""
    def __init__(self,valve):
        self.switch = mp.Event()
        self.dutyQue = mp.Queue()
        self.valve = Valve.Valve()
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
    def selfSync(self,syncTime):
        time.sleep(syncTime)
    def main(self):
        while self.swtich.is_set():

            if not self.dutyQue.empty():
                curDuty = self.dutyQue.get()
            onTime = curDuty/float(100)*pwmUnit/6000
            offTime = (100 - curDuty) / float(100) * pwmUnit/6000
            if onTime<0.005:
                self.valve.Off()
                continue
            self.valve.On()
            time.sleep(onTime)
            self.valve.Off()
            time.sleep(offTime)
            

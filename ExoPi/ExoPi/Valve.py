import gpiozero as gp
import time
class Valve(object):
    """description of class"""
    def __init__(self,name,index,valveRecQue,valveRecLock):
        self.name = name
        self.pin = gp.OutputDevice(index)
        self.valveRecQue = valveRecQue
        self.valveRecLock = valveRecLock
        self.onStr = ','+name + ',1'
        self.offStr = ','+name + ',0'
    def on(self):
        self.pin.on()
        curTime = time.time()
        with self.valveRecLock:
            self.valveRecQue.put(str(curTime)+self.onStr)
    def off(self):
        self.pin.off()
        curTime = time.time()
        with self.valveRecLock:
            self.valveRecQue.put(str(curTime)+self.offStr)



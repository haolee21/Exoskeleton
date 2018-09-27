import multiprocessing as mp
import time
import threading as th
import DataProcess as dp

class SenReader(object):
    """Read measurement from arduino"""
    # senLock: Lock to read share measurement
    # freq: Operating frequency
    def __init__(self,freq,senArray,senRecQue,senLock,port,sendPCQue,sendPCLock):
        self.port = port
        self.freq=freq
        self.senArray = senArray
        self.senRecQue = senRecQue
        self.senLock = senLock
        self.switch = mp.Event()
        self.sendPCQue = sendPCQue
        self.sendPCLock = sendPCLock

        self.mainProcess = mp.Process(target = self.main)
        
    def start(self):
        self.switch.set()
        self.mainProcess.start()
    def stop(self):
        self.switch.clear()
    def selfSync(self,sleepTime):
        time.sleep(sleepTime)
    def main(self):
        self.port.flushInput() #start brand new
        preInput = b'' #if current data is not complete, we wait until next cycle
        sendPCCount = 0
        while self.switch.is_set():
            selfSync = th.Thread(target= self.selfSync,args=(1/self.freq,))
            selfSync.start()
            if self.port.in_waiting>0:
                rawInput = preInput+self.port.read_until(b'\n')
                state,preInput,senStr = dp.dataSep(rawInput,self.senArray,self.senLock)
                if state:
                    self.senRecQue.put(self.senArray)
                    sendPCCount = sendPCCount+1
                    if sendPCCount ==4:
                        sendPCThread = th.Thread(target=self.sendPC,args=(senStr,))
                        sendPCThread.start()
                        sendPCCount = 0
                else:
                    pass
            else:
                print('get nothing')
            selfSync.join()
        print('Sensor ends')
    def sendPC(self,senStr):
        with self.sendPCLock:
            while not self.sendPCQue.empty():
                self.sendPCQue.get()
            self.sendPCQue.put(b'r'+senStr.encode('utf-8')+b'\n')

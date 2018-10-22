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
        self.period = 1/freq
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
        #preInput = b'' #if current data is not complete, we wait until next cycle
        sendPCCount = 0
        while self.switch.is_set():
            preTime = time.time()

            if self.port.in_waiting>0:

                rawInput = self.port.read_until(size=45)  #todo make data length a variable
                #state,preInput,senStr = dp.dataSep(rawInput,self.senArray,self.senLock)
                state,senStr = dp.dataSepSimp(rawInput,self.senArray,self.senLock,self.senRecQue)
                if state:
                    # tryTime1 = time.time()
                    #self.senRecQue.put(self.senArray) #self.senArray is a multiprocess manager.Array, cannot be directly put into a list, thus we transform it into a normal list
                                     # todo check the effect on speed
                    # tryTime2 = time.time()
                    # print(tryTime2-tryTime1)
                    sendPCCount = sendPCCount+1
                    if sendPCCount ==4:
                        sendPCThread = th.Thread(target=self.sendPC,args=(senStr,))
                        sendPCThread.start()
                        sendPCCount = 0
                else:
                    pass
            else:
                print('!get nothing')
            aftTime = time.time()
            while (aftTime - preTime)<self.period:
                aftTime = time.time()
                time.sleep(0.00001)

        print('#Sensor ends')
    def sendPC(self,senStr):
        with self.sendPCLock:
            while not self.sendPCQue.empty():
                self.sendPCQue.get()
            self.sendPCQue.put(senStr.encode('utf-8'))

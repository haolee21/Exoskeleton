import multiprocessing as mp
import time
import threading as th
import socket
class Client(object):
    """description of class"""
    def __init__(self,freq,pcIP,pcPort,sendPCQue,sendPCLock):
        self.switch = mp.Event()
        self.freq = freq
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.pcIP= pcIP
        self.pcPort = pcPort
        #
        print('connect to PC')
        self.sendPCQue = sendPCQue
        self.sendPCLock = sendPCLock
        self.mainProcess = mp.Process(target=self.main)

    def start(self):
        self.mainProcess.start()
    def stop(self):
        self.switch.clear()
    def selfSync(self,syncTime):
        time.sleep(syncTime)
    def main(self):
        self.socket.connect((self.pcIP, self.pcPort))
        self.switch.set()
        while self.switch.is_set():
            selfSync = th.Thread(target=self.selfSync,args=(1/self.freq,))
            selfSync.start()
            while not self.sendPCQue.empty():
                with self.sendPCLock:
                    self.socket.send(self.sendPCQue.get())
            # try:
            #     with self.sendPCLock:
            #         if self.sendPCQue.empty():
            #             self.socket.send(b'empty\n')
            #             pass
            #         else:
            #             self.socket.send(self.sendPCQue.get())
            #             self.socket.send(b'go\n')
            # except BrokenPipeError:
            #     pass
            selfSync.join()


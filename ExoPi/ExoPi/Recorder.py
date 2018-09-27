import multiprocessing as mp
import numpy as np
import os
class Recorder(object):
    """description of class"""
    def __init__(self,name,senRecQue,senName,valveRecQue,valveName,pwmRecQue,pwmName):
        self.name = name
        self.senRecQue = senRecQue
        self.senName = senName
        self.valveRecQue = valveRecQue
        self.valveName = valveName
        self.pwmRecQue = pwmRecQue
        self.pwmName = pwmName
        self.senData =mp.Manager().list()
        self.conData =mp.Manager().list()
        self.conSyncTime = None
        self.senSyncTime = None

    def conStart(self,syncTime):
        self.conSyncTime = syncTime
    def senStart(self,syncTime):
        self.senSyncTime = syncTime

    def saveData(self):
        senRecList =[]
        while not self.senRecQue.empty():
            senRecList.append(self.senRecQue.pop())
        valveRecList =[]
        while not self.valveRecQue.empty():
            valveRecList.append((self.valveRecQue.pop()))
        pwmRecList = []
        while not self.pwmRecQue.empty():
            pwmRecList.append(self.pwmRecQue.pop())

        os.mkdir('testData/'+self.name)
        np.savetxt('testData/'+self.name+'/'+self.name+'_sen.csv',self.senRecList,fmt='%d',delimiter=',',header=self.senName)
        np.savetxt('testData/' + self.name + '/' + self.name + '_valve.csv', self.valveRecList, fmt='%d', delimiter=',',
                   header=self.valveName)
        np.savetxt('testData/' + self.name + '/' + self.name + '_pwm.csv', self.pwmRecList, fmt='%d', delimiter=',',
                   header=self.pwmName)

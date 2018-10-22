import multiprocessing as mp
import numpy as np
import os
import queue
class Recorder(object):
    """description of class"""
    def __init__(self,name,senRecQue,senName,conRecQue,conRecName,pwmRecQue1,pwmRecQue2,pwmRecName,stateQue,syncTime):
        # data type:
                    # senRecQue: string of queue
                    # senName: single string
        self.name = name
        # Variable for recording sensing data
        self.senRecQue = senRecQue
        self.senName = senName
        # variables for recording control data
        self.conRecQue = conRecQue #.get() string time,valve name, state
        self.conRecName = conRecName # list(string) [val1,val2.....]
        # variables for recording pwm data
        self.pwmRecQue1 = pwmRecQue1
        self.pwmRecName = pwmRecName
        self.pwmRecQue2 = pwmRecQue2

        # variables for record sync of Pi and arduino
        self.syncTime = syncTime

        # variable for recording walking state
        self.stateQue = stateQue

        # self.valveRecQue = valveRecQue
        # self.valveName = valveName
        # self.pwmRecQue = pwmRecQue
        # self.pwmName = pwmName
        self.senData =mp.Manager().list()
        self.conData =mp.Manager().list()
        self.conSyncTime = None
        self.senSyncTime = None

    def conStart(self,syncTime):
        self.conSyncTime = syncTime
    def senStart(self,syncTime):
        self.senSyncTime = syncTime

    def saveData(self):
        print('#start to save data')
        senRecList =[]
        while not self.senRecQue.empty():
            # the data type in senRecQue is string "@time(7) sen1(4).........., all measurements has fixed length and are all integer
            dataList =[]
            recStr = self.senRecQue.get()
            dataList.append(int(recStr[1:8]))
            dataList.append(int(recStr[8:12]))
            dataList.append(int(recStr[12:16]))
            dataList.append(int(recStr[16:20]))
            dataList.append(int(recStr[20:24]))
            dataList.append(int(recStr[24:28]))
            dataList.append(int(recStr[28:32]))
            dataList.append(int(recStr[32:36]))
            dataList.append(int(recStr[36:40]))
            dataList.append(int(recStr[40:-1]))
            senRecList.append(dataList)



        # valve control data unpack
        preValCond = [0]*(len(self.conRecName) + 1) # this list also included time in the first element
        conRecList=[preValCond]

        #create valve name for saving
        valveName = 'Time,'+','.join(self.conRecName)
        while not self.conRecQue.empty():
            # the data type in conRecName is string
            # #time,name of valve,state
            curCon = self.conRecQue.get()
            # put curCon into list
            curList = []
            startI = 0
            # break recorded string into list time,name,condition

            while True:
                endI = curCon.find(',',startI)

                if endI==-1:
                    endI =len(curCon)

                    curList.append(curCon[startI:endI])
                    break
                else:
                    curList.append(curCon[startI:endI])
                    startI = endI+1
            # update current valve condition from previous valve condition
            # find the index for previous valve condition

            valI = self.conRecName.index(curList[1])+1 # the first element is time, so index +1 for valves

            preValCond[valI] = int(curList[2])
            preValCond[0]=int(float(curList[0])*1000+0.5)
            conRecList.append(preValCond.copy()) # list in python is pointer, thus if we exit the loop, the pointer will point back to original null data, which is 0

        # pwm data unpack
        prePwmCond = [0]*(len(self.pwmRecName)+1)
        pwmRecList = []
        #pwmRecList = [prePwmCond.copy()] # we don't need to record the first [0,0,0], pwmGen will do [time,0,0]
        pwmValName = 'Time,'+','.join(self.pwmRecName)

        allPwmList=[]
        while not self.pwmRecQue1.empty():
            curList=[]
            curPwm= self.pwmRecQue1.get()
            self.dataSep(curPwm,curList)
            allPwmList.append(curList.copy())
            # put curCon into list
            # break recorded string into list time,name,condition
            # update current valve condition from previous valve condition
            # find the index for previous valve condition
        while not self.pwmRecQue2.empty():
            curList=[]
            curPwm=self.pwmRecQue2.get()
            self.dataSep(curPwm,curList)
            allPwmList.append(curList.copy())
        # re-order all pwm data wrt time
        allPwmList.sort(key=lambda x:x[0])

        for curList in allPwmList:
            valI = self.pwmRecName.index(curList[1]) + 1  # the first element is time, so index +1 for valves
            prePwmCond[valI] = float(curList[2])
            prePwmCond[0] = int(float(curList[0]) * 1000 + 0.5)
            pwmRecList.append(
                prePwmCond.copy())  # list in python is pointer, thus if we exit the loop, the pointer will point back to original null data, which is 0


        # record sync time
        syncTimeList =[]
        while not self.syncTime.empty():
            syncTimeList.append(self.syncTime.get()*1000)

        stateList = []
        while not self.stateQue.empty():
            stateList.append(self.stateQue.get())

        os.mkdir('testData/'+self.name)
        np.savetxt('testData/'+self.name+'/'+self.name+'_sen.csv',senRecList,fmt='%d',delimiter=',',header=self.senName)
        np.savetxt('testData/' + self.name + '/' + self.name + '_valve.csv', conRecList, fmt='%d', delimiter=',',
                  header=valveName)
        np.savetxt('testData/'+self.name+'/'+self.name+'_sync.csv',syncTimeList,fmt='%f',header='SyncTime')
        np.savetxt('testData/' + self.name + '/' + self.name + '_pwm.csv', pwmRecList, fmt='%f', delimiter=',',
                   header=pwmValName)
        np.savetxt('testData/' + self.name + '/' + self.name + '_state.csv', stateList, fmt='%f', delimiter=',',
                   header='Time,State')
    def dataSep(self,dataStr,curList):
        startI = 0
        while True:
            endI = dataStr.find(',', startI)

            if endI == -1:
                endI = len(dataStr)
                curList.append(dataStr[startI:endI])
                break
            else:
                curList.append(dataStr[startI:endI])
                startI = endI + 1

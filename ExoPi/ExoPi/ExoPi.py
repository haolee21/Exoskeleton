import SenReader
import Setting
import Recorder
import PWMGen
import Client
import serial 
import threading as th
import multiprocessing as mp
import time
import ValveController as vc
port = serial.Serial(port='/dev/ttyACM0',baudrate=115200, parity=serial.PARITY_EVEN,stopbits=serial.STOPBITS_ONE)
if port.isOpen():
    port.close()
port.open()

time.sleep(0.1)


# Initialize Sensor
senManager = mp.Manager()
numOfSen = 9
senArray = senManager.Array('i',[0]*(numOfSen+1)) #+1 for time
senRecQue = mp.Queue()
sendPCQue = mp.Queue()
sendPCLock = mp.Lock()
senLock = mp.Lock()
sensor = SenReader.SenReader(125,senArray,senRecQue,senLock,port,sendPCQue,sendPCLock)
sensor.start()
# Initialize Controller
stateQue = mp.Queue()
valCondRecQue = mp.Queue()
cmdQue = mp.Queue()
cmdLock = mp.Lock()
valveRecQue = mp.Queue()
valveRecLock = mp.Lock()
syncTimeQue = mp.Queue()
        # pwm record queue
pwmRecQue1 = mp.Queue()
pwmRecQue2 = mp.Queue()

valveCon = vc.ValveController(100,100,cmdQue,cmdLock,senArray,senLock,valveRecQue,valveRecLock,syncTimeQue,pwmRecQue1,pwmRecQue2,stateQue)
valveCon.start()
print('# controller initialization finished')


# Initialize Client
exoClient = Client.Client(freq=60,pcIP='192.168.1.107',pcPort=12345,sendPCQue=sendPCQue,sendPCLock=sendPCLock)
exoClient.start()
# Initialize Recorder
name = input('Please input the name of this experiment:')
senName = 'Time,HipPos,KnePos,AnkPos,SyncPin,TankPre,KnePre,AnkPre,Test,Test'
conRecName = []
for val in valveCon.valveList:
    conRecName.append(val.name)
pwmRecName =[]
for pwmVal in valveCon.pwmValList:
    pwmRecName.append(pwmVal.name)
recorder = Recorder.Recorder(name=name, senRecQue=senRecQue,senName=senName,conRecQue=valveRecQue,conRecName=conRecName,syncTime=syncTimeQue,pwmRecQue1=pwmRecQue1,pwmRecQue2=pwmRecQue2,pwmRecName=pwmRecName,stateQue=stateQue)


def readCmd(cmdStr,cmdList):
    startI =0
    endI=0
    while endI!=len(cmdStr):
        endI = cmdStr.find(',',startI)
        if endI == -1:
            endI=len(cmdStr)
        cmdList.append(cmdStr[startI:endI])
        startI = endI+1

count = 0

while True:
    cmd=[]
    readCmd(input('Waiting for command:'),cmd) #todo we give command to controller throught SSH, need to change one day
    print(cmd)
    with cmdLock:
        cmdQue.put(cmd)
    if cmd == ['end']:
        sensor.stop()
        valveCon.stop()
        exoClient.stop()
        break
print('#end and start to save data')
recorder.saveData()
print('#Done data saving')
print('#Data save as '+name)

sensor.mainProcess.join()

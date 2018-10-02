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
senRecQue = senManager.Queue()
sendPCQue = senManager.Queue()
sendPCLock = senManager.Lock()
senLock = senManager.Lock()
sensor = SenReader.SenReader(125,senArray,senRecQue,senLock,port,sendPCQue,sendPCLock)
sensor.start()
# Initialize Controller
conManager = mp.Manager()
numOfVal =  4 #only count the low freq valve
valCondArray =conManager.Array('i',[0]*numOfVal)
valCondRecQue = conManager.Queue()
cmdQue = conManager.Queue()
cmdLock = conManager.Lock()
valveCon = vc.ValveController(100,100,cmdQue,cmdLock,senArray)
valveCon.start()
print('done valve')
# Initialize Client
exoClient = Client.Client(freq=60,pcIP='192.168.1.107',pcPort=12345,sendPCQue=sendPCQue,sendPCLock=sendPCLock)
exoClient.start()

print('check point 1')
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
print('end')


sensor.mainProcess.join()

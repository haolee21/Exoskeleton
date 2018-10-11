import time


def dataSep(msg, msgList):
    if len(msg)==45:
        if msg[0]=='@':
            msgList.append(int(msg[1:8]))
            msgList.append(int(msg[8:12]))
            msgList.append(int(msg[12:16]))
            msgList.append(int(msg[16:20]))
            msgList.append(int(msg[20:24]))
            msgList.append(int(msg[24:28]))
            msgList.append(int(msg[28:32]))
            msgList.append(int(msg[32:36]))
            msgList.append(int(msg[36:40]))
            msgList.append(int(msg[40:-1]))
            return True
        else:
            return False
    else:
        return False
def comSep(com, comList):  # This function will break command into list
    end = 0
    index = 0
    while end != -1:
        end = com.find('\n', index)
        if end == -1:
            break
        comList.append(com[index:end])
        index = end + 1
    comList.append(com[index:len(com)])


def getTime(sampTime, startPoint):
    # finalTime = [(t-startPoint)/1000 for t in sampTime]

    return (sampTime - startPoint) / 1000


def ProcessData(dataList, senList, procFunList):
    procIndex = 0
    for i in senArray:
        # important, senArray index start from 1
        dataList[i - 1] = procFunList[procIndex](dataList[i - 1])
        procIndex = procIndex + 1


DEG90MEA = 789.33
DEG135MEA = 921.33
CROSSSECAREA = 64.516  # unit in mm


def getPre(meaVolt):
    """this return pressure in psi"""
    return meaVolt * 250 / 1024 - 25


def getAng(meaVolt, offset):
    """this function return in degree"""
    curVolt = 0.0049 * meaVolt
    curAng = curVolt / 4.918 * 340 - offset
    return curAng


def getVolt(meaVolt):
    return 0.0049 * meaVolt


def getOri(mea):
    return mea
import time


def MsgSep(msg, msgList):
    while not len(msg) == 0:
        beg = -1
        beg1 = msg.find(b'r') + 1
        beg2 = msg.find(b'con') + 1
        if (beg1 < beg2) and (beg1 != -1):
            beg = beg1
        elif (beg2 < beg1) and (beg2 != -1):
            beg = beg2

        end = msg.find(b'\n', beg)
        if end == -1:
            break
        msgList.append(msg[beg:end])
        msg = msg[end + 1:]
        if beg == -1:
            return False
    return True


def comSep(com, comList):  # This function will break command into list
    end = 0
    index = 0
    while end != -1:
        end = com.find(',', index)
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
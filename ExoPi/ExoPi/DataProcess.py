import time
def dataSep(strOri,senArray,senLock):
    '''Inputs string read from serial port and return 'status','data left',data'''
    startI = strOri.rfind(b'@')+1
    endI = strOri.rfind(b'\n',startI)
    if startI==0: #-1+1 = 0
        return False,b'',b''
    elif endI ==-1:
        return False,strOri[startI:],b''  #if has no start, wait for more data
                                         # if has no end but has start, storage the last data and wait
    meaStr = strOri[startI:endI]
    meaStr=meaStr.decode('utf-8')
    senStartI =0
    i = 0
    while True:
        senEndI = meaStr.find(',',senStartI)
        if senEndI ==-1:
            break
        try:
            with senLock:
                senArray[i] = int(meaStr[senStartI:senEndI])
        except (ValueError,IndexError,OverflowError):#TODO need to know what cause the exception
            #print('Sensor received wrong data')
            return False,b'',b''
            break
        senStartI = senEndI +1
        i=i+1
    # return the rest of sensing data
    if endI+1 == len(strOri):
        return True,b'',meaStr
    else:
        return True,strOri[endI:],meaStr
    



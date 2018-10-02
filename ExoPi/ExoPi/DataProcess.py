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
    senEndI =0
    while senEndI!=len(meaStr):
        senEndI = meaStr.find(',',senStartI)
        if senEndI ==-1:
            senEndI =len(meaStr)
        try:
            with senLock:
                senArray[i] = int(meaStr[senStartI:senEndI])
                #senArray[i]=meaStr[senStartI:senEndI]
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
    

def dataSepSimp(curStr,senArray,senLock):
    if len(curStr)==36:
        try:
            curSen = curStr.decode('utf-8')
            with senLock:
                senArray[0]=int(curSen[1:8])
                senArray[1]=int(curSen[8:11])
                senArray[2]=int(curSen[11:14])
                senArray[3]=int(curSen[14:17])
                senArray[4]=int(curSen[17:20])
                senArray[5]=int(curSen[20:23])
                senArray[6]=int(curSen[23:26])
                senArray[7]=int(curSen[26:29])
                senArray[8]=int(curSen[29:32])
                senArray[9]=int(curSen[32:-1])

                #senArray[:] = [int(curSen[1:8]),int(curSen[8:11]),int(curSen[11:14]),int(curSen[14:17]),int(curSen[17:20]),int(curSen[20:23]),int(curSen[23:26]),int(curSen[26:29]),int(curSen[29:32]),int(curSen[32:-1])]
        except (ValueError, IndexError, OverflowError):  # TODO need to know what cause the exception
            return False,''
        return True,curSen
    else:
        return False,''
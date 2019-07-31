import socket
import sys
import time
import Display as dp
import numpy as np
def main():

    HOST = '192.168.1.142' # Server IP or Hostname
    PORT = 12345 # Pick an open Port (1000+ recommended), must match the client sport
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('Socket created')
    #managing error exception

    try:
	    s.bind((HOST, PORT))
    except socket.error:
	    print ('Bind failed ')
    s.listen()

    print ('Socket awaiting messages')
    conn, addr = s.accept()
    print ('Connected')


    ylabel_sen=['volt','volt','volt','volt','volt','volt','volt','volt','volt']
    ylim_sen=[(0,1000),(0,1000),(0,1000),(0,1000),(0,1000),(0,1000),(0,1000),(0,1000),(0,1000)]
    title_sen=['title1','title2','title3','title4','title5','title6','title7','title8','title9']
    graph_sen = dp.Plotter(tTot=2,sampF=615/5,figNum=8,yLabelList=ylabel_sen,yLimList=ylim_sen,titleList=title_sen)

    ylabel_val=['on','on','on','on','on','on']
    ylim_val=[(33,101),(33,101),(33,101),(33,101),(33,101),(33,101)]
    title_val=['val1','val2','val3','val4','val5','val6']
    graph_val=dp.Plotter(tTot=2,sampF=615/5,figNum=6,yLabelList=ylabel_val,yLimList=ylim_val,titleList=title_val)    

    DATALEN=20
    VALNUM=6
    while True:
        data = conn.recv(DATALEN+VALNUM)
        senData=[]
        valData=[]
        if(int(data[0])==64):
            for i in range(1,DATALEN-1,2): #remove @ and \n
                senData.append(int(data[i+1]<<8)+int(data[i]))

            for i in range(DATALEN,VALNUM+DATALEN,1):
                valData.append(int(data[i]))
            

            senData = np.array(senData)
            valData = np.array(valData)
            graph_sen.UpdateFig(senData)
            graph_val.UpdateFig(valData)
        # if(data =='end'):
        #     print(data=='end')
        #     break
    
    



        

if __name__ == '__main__':
    main()
    
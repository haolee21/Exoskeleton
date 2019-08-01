import socket
import sys
import time
import Display as dp
import numpy as np


def main():

    HOST = '192.168.1.142'  # Server IP or Hostname
    # Pick an open Port (1000+ recommended), must match the client sport
    PORT = 12345
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('Socket created')
    # managing error exception

    try:
        s.bind((HOST, PORT))
    except socket.error:
        print('Bind failed ')
    s.listen()

    print('Socket awaiting messages')
    conn, addr = s.accept()
    print('Connected')

    NUMSEN=16
    DATALEN = NUMSEN*2+2
    VALNUM = 6
    PWMNUM = 2
    sampFreq = 615/10
    ylabel_sen = ['volt', 'volt', 'volt', 'volt','volt','volt','volt','volt',
                  'volt', 'volt', 'volt', 'volt', 'volt','volt','volt','volt']
    ylim_sen = [(0, 1000), (0, 1000), (0, 1000), (0, 1000),(0, 1000),(0, 1000),(0, 1000),(0, 1000),
                (0, 1000), (0, 1000), (0, 1000), (0, 1000), (0, 1000),(0, 1000),(0, 1000),(0, 1000)]
    title_sen = ['title1', 'title2', 'title3', 'title4', 'title5', 'title6', 'title7', 'title8',
                 'title9','title10','title11', 'title12', 'title13', 'title14', 'title15', 'title16',]
    graph_sen = dp.Plotter(tTot=2, sampF=sampFreq, figNum=NUMSEN,
                           yLabelList=ylabel_sen, yLimList=ylim_sen, titleList=title_sen)

    ylabel_val = ['on', 'on', 'on', 'on', 'on', 'on']
    ylim_val = [(33, 101), (33, 101), (33, 101),
                (33, 101), (33, 101), (33, 101)]
    title_val = ['val1', 'val2', 'val3', 'val4', 'val5', 'val6']
    graph_val = dp.Plotter(tTot=2, sampF=sampFreq, figNum=VALNUM,
                           yLabelList=ylabel_val, yLimList=ylim_val, titleList=title_val)

    ylabel_pwm = ['Duty (%)', 'Duty (%)', 'Duty (%)', 'Duty (%)']
    ylim_pwm = [(0, 100), (0, 100), (0, 100), (0, 100)]
    title_pwm = ['KnePre', 'AnkPre', 'No use', 'No use']
    graph_pwm = dp.Plotter(tTot=2, sampF=sampFreq, figNum=4,
                           yLabelList=ylabel_pwm, yLimList=ylim_pwm, titleList=title_pwm)
    
    while True:
        data = conn.recv(DATALEN+VALNUM+PWMNUM)
        senData = []
        valData = []
        pwmData = []
        try:
            if(int(data[0]) == 64):
                for i in range(1, DATALEN-1, 2):  # remove @ and \n
                    senData.append(int(data[i+1] << 8)+int(data[i]))

                for i in range(DATALEN, VALNUM+DATALEN, 1):
                    valData.append(int(data[i]))
                for i in range(DATALEN+VALNUM, DATALEN+VALNUM+PWMNUM, 1):
                    pwmData.append(int(data[i]))

                senData = np.array(senData)
                valData = np.array(valData)
                pwmData = np.array(pwmData)

                start=time.time()
                graph_sen.UpdateFig(senData)
                graph_val.UpdateFig(valData)
                graph_pwm.UpdateFig(pwmData)
                end = time.time()
                print('total time: ',end-start)
        except IndexError:
            print('socket ends')
            break


if __name__ == '__main__':
    main()

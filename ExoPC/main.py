import socket
import sys
import time
import Display as dp
import numpy as np
import paramiko
import datetime
from numba import jit

def main():
    #sync time with pc
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.connect('192.168.1.134',username='pi',password='bionics')
    ssh.exec_command('sudo timedatectl set-time \''+ str(datetime.datetime.now())+'\'')
    ssh.close()

    #create tcp server
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
    s.listen(5) #this number I believe determine how low we are going to connect to this socket

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
    title_sen = ['LAnkPos', 'LKnePos', 'LHipPos', 'RAnkPos', 'RKnePos', 'RHipPos', 'title7', 'title8',
                 'TankPre','LKnePre','LAnkPre', 'title12', 'title13', 'title14', 'title15', 'title16',]
    graph_sen = dp.Plotter(tTot=2, sampF=sampFreq, figNum=NUMSEN,
                           yLabelList=ylabel_sen, yLimList=ylim_sen, titleList=title_sen)

    ylabel_val = ['on', 'on', 'on', 'on', 'on', 'on']
    ylim_val = [(33, 101), (33, 101), (33, 101),
                (33, 101), (33, 101), (33, 101)]
    title_val = ['LKneVal1', 'LKneVal2', 'LAnkVal1', 'LAnkVal2', 'LBalVal', 'LRelVal']
    graph_val = dp.Plotter(tTot=2, sampF=sampFreq, figNum=VALNUM,
                           yLabelList=ylabel_val, yLimList=ylim_val, titleList=title_val)

    ylabel_pwm = ['Duty (%)', 'Duty (%)', 'Duty (%)', 'Duty (%)']
    ylim_pwm = [(0, 100), (0, 100), (0, 100), (0, 100)]
    title_pwm = ['KnePre', 'AnkPre', 'No use', 'No use']
    graph_pwm = dp.Plotter(tTot=2, sampF=sampFreq, figNum=4,
                           yLabelList=ylabel_pwm, yLimList=ylim_pwm, titleList=title_pwm)
    # start = time.time()
    gotOne = True
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

                # g_start = time.time()
                if gotOne:
                    graph_sen.UpdateFig(senData)
                    graph_val.UpdateFig(valData)
                    graph_pwm.UpdateFig(pwmData)      
                    # g_end = time.time()
                    gotOne=False
                else:
                    gotOne=True
                # print("graph time: ",g_end-g_start)
        except IndexError:
            print('socket ends')
            break
        # end = time.time()
        # print("duration: ",end-start)
        # start = end 


if __name__ == '__main__':
    main()

import socket
import sys
import time
import Display as dp
import numpy as np
import paramiko
import datetime

import multiprocessing as mp

NUMSEN=16
DATALEN = NUMSEN*2+2
VALNUM = 6
PWMNUM = 2
sampFreq = 615/20
def main():
    #sync time with pc
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.connect('192.168.1.136',username='pi',password='bionics')
    ssh.exec_command('sudo timedatectl set-time \''+ str(datetime.datetime.now())+'\'')
    ssh.close()
    print('sudo timedatectl set-time \''+ str(datetime.datetime.now())+'\'')
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

    ylabel_sen = ['volt', 'volt', 'volt', 'volt','volt','volt','volt','volt',
                  'volt', 'volt', 'volt', 'volt', 'volt','volt','volt','volt']
    ylim_sen = [(0, 1000), (0, 1000), (0, 1000), (0, 1000),(0, 1000),(0, 1000),(0, 1000),(0, 1000),
                (0, 600), (0, 600), (0, 600), (0, 600), (0, 600),(0, 600),(0, 600),(0, 600)]
    title_sen = ['LHipPos', 'LKnePos', 'LAnkPos', 'RHipPos', 'RKnePos', 'RAnkPos', 'title7', 'title8',
                 'TankPre','LKnePre','LAnkPre', 'RKnePre', 'RAnkPre', 'title14', 'title15', 'title16',]
    graph_sen = dp.Plotter(tTot=1, sampF=sampFreq, figNum=NUMSEN,
                           yLabelList=ylabel_sen, yLimList=ylim_sen, titleList=title_sen)

    ylabel_val = ['on', 'on', 'on', 'on', 'on', 'on']
    ylim_val = [(33, 101), (33, 101), (33, 101),
                (33, 101), (33, 101), (33, 101)]
    title_val = ['LKneVal1', 'LKneVal2', 'LAnkVal1', 'LAnkVal2', 'LBalVal', 'LRelVal']
    graph_val = dp.Plotter(tTot=1, sampF=sampFreq, figNum=VALNUM,
                           yLabelList=ylabel_val, yLimList=ylim_val, titleList=title_val)

    ylabel_pwm = ['Duty (%)', 'Duty (%)', 'Duty (%)', 'Duty (%)']
    ylim_pwm = [(0, 100), (0, 100), (0, 100), (0, 100)]
    title_pwm = ['KnePre', 'AnkPre', 'No use', 'No use']
    graph_pwm = dp.Plotter(tTot=1, sampF=sampFreq, figNum=4,
                           yLabelList=ylabel_pwm, yLimList=ylim_pwm, titleList=title_pwm)
    # start = time.time()
    
    while True:
        data = conn.recv(DATALEN+VALNUM+PWMNUM)
        
        try:
            if(int(data[0]) == 64):
                
                # senP = mp.Process(target=UpdateGraph,args=(graph_sen,data,SenHandle))
                # valP = mp.Process(target=UpdateGraph,args=(graph_val,data,ValveHandle))
                # pwmP = mp.Process(target=UpdateGraph,args=(graph_pwm,data,PWMHandle))
                # senP.start()
                # valP.start()
                # pwmP.start()

                # senP.join()
                # valP.join()
                # pwmP.join()
                # start = time.time()
                UpdateGraph(graph_sen,data,SenHandle)
                UpdateGraph(graph_val,data,ValveHandle)
                UpdateGraph(graph_pwm,data,PWMHandle)
                # end = time.time()
                # print(end-start)
            
                # print("graph time: ",g_end-g_start)
        except IndexError:
            print('socket ends')
            break
        # end = time.time()
        # print("duration: ",end-start)
        # start = end 
def SenHandle(data):
    senData=[]
    for i in range(1, DATALEN-1, 2):  # remove @ and \n
        senData.append(int(data[i+1] << 8)+int(data[i]))
    return np.array(senData)
def ValveHandle(data):
    valData=[]
    for i in range(DATALEN, VALNUM+DATALEN, 1):
        valData.append(int(data[i]))
    return np.array(valData)
def PWMHandle(data):
    pwmData=[]
    for i in range(DATALEN+VALNUM, DATALEN+VALNUM+PWMNUM, 1):
        pwmData.append(int(data[i]))
    return np.array(pwmData)
def UpdateGraph(graph,data,dataHandle):
    graph.UpdateFig(dataHandle(data))

if __name__ == '__main__':
    main()

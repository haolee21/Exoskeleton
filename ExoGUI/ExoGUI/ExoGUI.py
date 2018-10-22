import tkinter as tk
import RecSpring
import Connector
import IP_set
import SSH_set
import TestFun
import time
import threading as th
import paramiko
import queue
import pickle
import socket
import matplotlib.pyplot as plt
import numpy as np
import Plotter
import multiprocessing as mp
import DataProcess as dp

class MainWindow(object):
    def __init__(self):
        # Program
        self.plotData = queue.Queue()
        self.plotLock = th.Lock()
        self.recData = queue.Queue()
        self.recDataLock = th.Lock()
        self.s = None

        # share ssh components
        self.ssh = None
        self.sshin = None
        self.sshConnect = th.Event()

        # share TCP/IP components
        self.tcpipConnect = th.Event()
        # GUI
        self.window = tk.Tk()
        self.window.title('Bionics Lower Limb Exoskeleton')
        self.window.geometry('800x600')
        #Public Variable that will display

        # data plotting components
        self.startPlot = th.Event()



        # load previous setting
        try:
            preSetLink = open('ExoGUI_preset.exo')
            preSetDic = preSetLink.readline()
            loadSet = open(preSetDic, 'rb')
            self.exo1=pickle.load(loadSet)
            loadSet.close()
            self.exo1.status= False
        except IOError:
            print('Does not load any setting')
            self.exo1 = Connector.Connector()
        self.menuBar = tk.Menu(self.window)

        self.fileMenu = tk.Menu(self.menuBar, tearoff=0)
        self.menuBar.add_cascade(label='File', menu=self.fileMenu)
        self.fileMenu.add_command(label='Save Setting',command=self.exo1.saveSetting)
        self.fileMenu.add_command(label='Load Setting',command=self.exo1.loadSetting)

        self.connectMenu = tk.Menu(self.menuBar,tearoff=0)
        self.menuBar.add_cascade(label='Connection',menu=self.connectMenu)
        self.connectMenu.add_command(label='IP Setting',command= lambda  curSet = self.exo1,root=self: IP_set.IP_set(curSet,root))
        self.connectMenu.add_command(label='SSH Setting',command=lambda curSet = self.exo1,root=self:SSH_set.SSH_set(curSet,root))


        self.testMenu = tk.Menu(self.menuBar,tearoff = 0)
        self.menuBar.add_cascade(label='Test',menu=self.testMenu)
        self.testMenu.add_command(label='Test Low Freq. Valve',command = TestFun.testFun)
        self.testMenu.add_command(label='Test Sync.',command = TestFun.testFun)
        self.testMenu.add_command(label='Test Encoder',command=TestFun.testFun)

        self.functionMenu = tk.Menu(self.menuBar,tearoff=0)
        self.menuBar.add_cascade(label='Function',menu=self.functionMenu)
        self.functionMenu.add_command(label='Spring Mode',command = lambda root = self.window:RecSpring.RecSpring(root))


        self.connectButText = tk.StringVar(value='Connect')

        # main layout

        # 1. Connect to the Exo

        self.curIPText = tk.Label(self.window,text = 'Exo IP')
        self.curIPText.grid(row=1,column=1,padx=10)
        self.curIPText.config(font=("Courier",12))
        self.curIP = tk.Label(self.window,text = self.exo1.exoIP)
        self.curIP.grid(row=1,column=2,padx=40)
        self.connectStatusLab = tk.Label(self.window,bg='red',text='Not Connected',font=('Courier',12),padx=50)
        self.connectStatusLab.grid(row=1,column=4)
        self.connectButton = tk.Button(self.window,command=lambda label = self.connectStatusLab:self.connectExo(label),textvariable=self.connectButText,padx=50)
        self.connectButton.grid(row=1,column=3)
        self.connectButton.config(font=('Courier',12))

        #2 Incoming data
        self.showRecText = tk.Text(master=self.window)
        self.showRecText.grid(row=2,column=1,columnspan=4,rowspan=10)
        self.showRecText.insert(tk.END,'')


        self.window.config(menu=self.menuBar)
        self.window.after(100,self.refreshRead)

        #3 Plot the data
        self.buttonPlot = tk.Button(master = self.window,command = self.dataPlot,text = 'Plot Measurement')
        self.buttonPlot.grid(row=13,columnspan=1,column=2)
        self.b_stopPlot = tk.Button(master = self.window,command=self.stopPlot,text = 'Stop Plotting')
        self.b_stopPlot.grid(row=13,columnspan=1,column=3)
        #4 Test function

        self.buttonTestFun = tk.Button(master = self.window,command=self.testFun,text='Test Fun')
        self.buttonTestFun.grid(row=14,columnspan=2,column=1)
        self.buttonEndTestFun = tk.Button(master=self.window,command=self.endTestFun,text='End Test Fun')
        self.buttonEndTestFun.grid(row=14,columnspan=2,column=3)
    def connectExo(self, label): #this button also deal with disconnecting
        if self.exo1.status:
            label.config(bg='red', text='Disconnected')
            self.connectButText.set('Connect')
            self.exo1.disconnect()
            self.sshin.channel.sendall('end\n')
            time.sleep(3) # for exo to return some turning off message

            self.startPlot.clear()
            self.sshConnect.clear()
            self.tcpipConnect.clear()
            self.ssh.close()
            self.s.close()
        else:
            self.sshConnect.set()
            self.tcpipConnect.set()
            sshThread = th.Thread(target=self.set_SSHConnect)
            sshThread.start()
            ipThread = th.Thread(target=self.connectIP)
            ipThread.start()
            self.exo1.connect()
            label.config(bg='green', text='Connected')
            self.connectButText.set('Disconnect')


    def set_SSHConnect(self):
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect('192.168.1.134', 22, 'pi', 'bionics') #todo need to fix why ssh_set is not working
        self.sshin,sshout, _ = self.ssh.exec_command('python3 Exo/ExoPi/ExoPi.py',get_pty=True)

        # sshin, sshout, _ = self.ssh.exec_command('python3 test.py', get_pty=True)

        # count = 1
        while self.sshConnect.is_set():
            if sshout.channel.recv_ready():
                with self.recDataLock:
                    self.recData.put(sshout.channel.recv(1024))

            # time.sleep(0.5)
            # count = count+1
            # if count ==10:
            #     self.sshin.channel.sendall('\x03') # the ctrl c command to terminate the session






    def refreshRead(self):
        while not self.recData.empty():
            with self.recDataLock:
                 self.showRecText.insert(tk.END,self.recData.get())
        self.window.after(10,self.refreshRead)

    def connectIP(self):

        self.s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        try:
            self.s.bind((self.exo1.pcIP,self.exo1.ipPort))
        except socket.error:
            with self.recDataLock:
                self.recData.put('IP Bind Failed\n')
        self.s.listen(5)
        conIP,addr=self.s.accept()
        with self.recDataLock:
            self.recData.put('TCP/IP Bind success\n')

        while self.tcpipConnect.is_set():
            data = conIP.recv(1024)
            print(data)
            cmdList = []
            dp.comSep(data.decode('utf-8'),cmdList)
            for cmd in cmdList:
                dataList = []
                dp.dataSep(cmd,dataList)
                with self.plotLock:
                    self.plotData.get()
                    self.plotData.put(dataList)

            time.sleep(0.05)
    def plotBut(self):
        thread = th.Thread(target=self.testPlot)
        thread.start()

    def testPlot(self):
        fig,ax = plt.subplots(1,1)
        x = np.random.rand(30)
        y = np.random.rand(30)
        plt.show(block=False)
        line, = ax.plot(x, y)
        fig.canvas.draw()
        ax.draw_artist(ax.patch)
        ax.draw_artist(line)

        while True:
            yData = np.random.rand(30)
            line.set_ydata(yData)
            ax.draw_artist(ax.patch)
            ax.draw_artist(line)
            fig.canvas.update()
            fig.canvas.flush_events()
            time.sleep(0.01)
    def testFun(self):
        self.sshin.channel.sendall('test,start\n')
    def endTestFun(self):
        self.sshin.channel.sendall('test,stop\n')
    def dataPlot(self):
        dataLen=100
        senArray = [1,2,3]
        #sensorTypeArray = [dp.getOri,dp.getOri,dp.getOri]
        tQue = mp.Queue()
        yQue = mp.Queue()
        #todo check the definition of plotter
        titleArray = ['plot1','plot2','plot3']
        yLimArray =[(0,300),(0,300),(0,300)]
        yLabelArray=['test','test','test']

        sensorPlot = Plotter.Plotter(tQue,yQue,self.plotLock,len(yLabelArray),dataLen,senArray,titleArray,yLimArray,yLabelArray)
        #todo fix the plotter reference problem
        while self.startPlot.is_set():
            while not self.recData.empty():
                with self.recDataLock:
                    curData = self.plotData.get()
                    yQue.put(curData[2:])
                    tQue.put(curData[1])
    def stopPlot(self):
        self.startPlot.clear()
    def voidFun(self):
        print('execute void function')






if __name__ == "__main__":
    app = MainWindow()
    app.window.mainloop()
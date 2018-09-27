import pickle
from tkinter import filedialog
import socket


class Connector(object):
    """description of class"""

    curPath = ''
    sshUName=''
    sshPass=''
    pcIP = socket.gethostbyname(socket.gethostname())
    exoIP=''
    exoMainFile=''
    ipPort = 12345
    def __init__(self):
        self.status = False


    def loadSetting(self):
        filePath = filedialog.askopenfilename()
        loadSet = open(filePath, 'rb')

        loadSet.close()
        self.__dict__.update(loadSet)

    def saveSetting(self):
        file = filedialog.asksaveasfilename(defaultextension='.exo')
        #file = filedialog.asksaveasfile(mode='w',defaultextension='.exo')
        curSet = open(file,'wb')
        pickle.dump(self,curSet)
        self.curPath = file
        curSet.close()
        defaultFile = open('ExoGUI_preset.exo','w')
        defaultFile.write(file)
        defaultFile.close()
        print(self.sshUName)
        print(self.sshPass)
    def connect(self):
        self.status=True
    def disconnect(self):
        self.status=False

import tkinter as tk
import Connector
class SSH_set(object):
    def __init__(self,curSet,root):
        self.root = root
        self.curSet = curSet
        self.curSet = Connector.Connector()
        self.sshWindow = tk.Toplevel()
        self.sshWindow.geometry('400x200')
        self.sshWindow.title('SSH Setting')
        self.sshUNameText=\
            tk.Label(self.sshWindow,text='SSH User Name').grid(row=1,column=1)
        self.sshUNameEntry = tk.Entry(master = self.sshWindow)
        self.sshUNameEntry.grid(row=1,column=3)
        self.sshUNameEntry.insert(string=curSet.sshUName,index=0)

        self.sshPassText= tk.Label(self.sshWindow,text='SSH Password').grid(row=2,column=1)
        self.sshPassEntry = tk.Entry(master = self.sshWindow)
        self.sshPassEntry.grid(row=2,column=3)
        self.sshPassEntry.insert(string=self.curSet.sshPass,index=0)

        self.exoFileText = tk.Label(self.sshWindow,text='Exo Main File Location').grid(row=3, column=1)
        self.exoFileEntry=tk.Entry(master=self.sshWindow)
        self.exoFileEntry.grid(row=3,column=3)
        self.exoFileEntry.insert(string=self.curSet.exoMainFile,index=0)

        self.confirmBut = tk.Button(self.sshWindow,text='Confirm',command=self.ssh_confirm)\
            .grid(row=4,column=1)
        self.cancelBut= tk.Button(self.sshWindow,text='Cancel',command=self.sshWindow.destroy).grid(row=4,column=3)
        self.sshWindow.mainloop()
    def ssh_confirm(self):
        self.curSet.sshUName = self.sshUNameEntry.get()
        self.curSet.sshPass = self.sshPassEntry.get()
        self.sshWindow.destroy()
        print(self.curSet.sshUName)
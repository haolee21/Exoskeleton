import tkinter as tk


class IP_set(object):
    def __init__(self,curSet,mainWindow):
        self.mainWindow = mainWindow
        self.curSet = curSet
        self.ipWindow = tk.Toplevel()
        self.ipWindow.geometry('300x200')
        self.ipWindow.title('IP Setting')
        self.pcIPText = tk.Label(self.ipWindow, text='PC IP').grid(row=1, column=1)
        self.pcIPEntry = tk.Entry(master=self.ipWindow)
        self.pcIPEntry.grid(row=1, column=2)
        self.pcIPEntry.insert(string=curSet.pcIP, index=0)

        self.exoIPText = tk.Label(self.ipWindow, text='Exo IP').grid(row=2, column=1)
        self.exoIPEntry = tk.Entry(master=self.ipWindow)
        self.exoIPEntry.grid(row=2, column=2)
        self.exoIPEntry.insert(string=curSet.exoIP, index=0)
        self.confirmBut = tk.Button(self.ipWindow, text='Confirm', command=self.ip_confirm).grid(row=3,column=1)
        self.cancelBut = tk.Button(self.ipWindow, text='Cancel', command=self.ipWindow.destroy).grid(row=3,column=2)



        self.ipWindow.mainloop()

    def ip_confirm(self):
        self.curSet.pcIP=self.pcIPEntry.get()
        self.curSet.exoIP=self.exoIPEntry.get()
        self.mainWindow.curIP.config(text=self.curSet.pcIP)
        self.ipWindow.destroy()
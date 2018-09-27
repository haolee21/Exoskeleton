import tkinter as tk


def ip_confirm():

    return 0


def ip_setWindow(preSet):
    ipWindow = tk.Toplevel()
    ipWindow.geometry('400x300')
    ipWindow.title('IP Setting')
    pcIPText = tk.Label(ipWindow,text = 'PC IP').grid(row = 1,column=1)
    pcIPEntry =tk.Entry(master = ipWindow)
    pcIPEntry.grid(row=1,column=2)
    pcIPEntry.insert(string='192.168.1.134',index=0)
    exoIPText = tk.Label(ipWindow,text = 'Exo IP').grid(row =2,column=1)
    exoIPEntry = tk.Entry(master=ipWindow)
    exoIPEntry.grid(row=2,column=2)
    exoIPEntry.insert(string='192.168.1.134',index=0)

    confirmBut = tk.Button(ipWindow,text = 'Confirm',command = ip_confirm).grid(row=3,column=1)
    cancelBut = tk.Button(ipWindow,text = 'Cancel',command =ipWindow.destroy).grid(row=3,column=2)
    
    
    ipWindow.mainloop()

def sPortSetWindow():
    sPortWindow = tk.Toplevel()
    sPortWindow.geometry('400x300')
    sPortWindow.title('Arduino Port Setting')
    senPort1Text = tk.Label(sPortWindow,text='Arduino 1:').grid(row=1,column=1)
    senPort1Entry = tk.Entry(master=sPortWindow)

import numpy as np
import matplotlib.pyplot as plt
import time
# reference: https://bastibe.de/2013-05-30-speeding-up-matplotlib.html
class Plotter(object):
    def __init__(self,tTot,sampF,figNum,yLabelList,yLimList,titleList):
        self.ncols = int(np.floor(np.sqrt(figNum)))
        self.nrows = int(np.round(figNum/self.ncols))
        #initial data
        self.time = np.linspace(start =0,stop=tTot,num=np.round(tTot*sampF))
        rowData = np.zeros(len(self.time))
        self.totalData=[]
        for i in range(self.nrows*self.ncols):
            self.totalData.append(rowData)
        if(figNum<self.ncols*self.nrows):
            for i in range(self.ncols*self.nrows-figNum):
                yLabelList.append('#not used#')
                yLimList.append((-10,100))
                titleList.append('#not used#')



        # create figure
        self.fig,self.ax = plt.subplots(ncols=self.ncols,nrows=self.nrows)
        # if(_ncols*_nrows-figNum>2):
        #     print("figNum issue")
        # elif(_ncols*_nrows>figNum):
        #     self.fig.delaxes(self.ax[_nrows-1,_ncols-1])
        plt.show(block=False)
        # create lines that can be update later
        self.lineList =[]
        idx =0
        for row in range(self.nrows):
            for col in range(self.ncols):
                line,=self.ax[row,col].plot(self.time,self.totalData[idx])
                self.lineList.append(line)
                self.fig.canvas.draw()
                self.ax[row,col].draw_artist(self.ax[row,col].patch)
                self.ax[row,col].draw_artist(line)
                self.ax[row,col].set_title(titleList[idx])
                self.ax[row,col].set_ylim(yLimList[idx])
                self.ax[row,col].set_xlabel('Time')
                self.ax[row,col].set_ylabel(yLabelList[idx])
                idx = idx+1
        
    
    def UpdateFig(self,_data):
        if(len(_data)<self.ncols*self.nrows):
            _data = np.append(_data,np.zeros(self.ncols*self.nrows-(len(_data))))
        idx2 = 0
        # curTIme = time.time()
        for row in range(self.nrows):
            for col in range(self.ncols):
                
                self.totalData[idx2] = self.totalData[idx2][1:]
                self.totalData[idx2] = np.append(self.totalData[idx2],_data[idx2])
                
                
                self.lineList[idx2].set_ydata(self.totalData[idx2])
                self.ax[row,col].draw_artist(self.ax[row,col].patch)
                self.ax[row,col].draw_artist(self.lineList[idx2])
                # self.fig.canvas.update()
                #self.fig.canvas.flush_events()
                
                idx2 = idx2+1   
        # endTime = time.time()
        # print(endTime-curTIme)
        self.fig.canvas.update()
        # self.fig.canvas.draw()
        self.fig.canvas.flush_events()

    
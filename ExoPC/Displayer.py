import numpy as np
import matplotlib.pyplot as plt
class Displayer(object):
    def __init__(self,tTot,sampF,figNum,yLabelList,yLimList,titleList):
        self.ncols = np.floor(np.sqrt(figNum))
        self.nrows = np.round(figNum/self.ncols)
        #initial data
        self.time = np.linspace(start =0,stop=tTot,num=np.round(tTot*sampF))
        rowData = np.zeros()
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
        index =0
        for row in range(self.nrows):
            for col in range(self.ncols):
                line,=self.ax[row,col].plot(self.time,self.totalData[index])
                self.lineList.append(line)
                self.fig.canvas.draw()
                self.ax[row,col].draw_artist(self.ax[row,col].patch)
                self.ax[row,col].draw_artist(line)
                self.ax[row,col].set_title(titleList[index])
                self.ax[row,col].set_ylim(yLimList[index])
                self.ax[row,col].set_xlabel('Time')
                self.ax[row,col].set_ylabel(yLabelList[index])
                index = index+1
        return 0
    def UpdateFig(self,_data):
        if(len(_data)<self.ncols*self.nrows):
            _data = np.append(_data,np.zeros(self.ncols*self.nrows-(len(_data)))
        index = 0
        for row in range(self.nrows):
            for col in range(self.ncols):
                self.totalData[index] = self.totalData[index][1:]
                self.totalData[index] = np.append(self.totalData[index],_data[index])
                self.lineList[index].set_ydata(self.totalData[index])
                self.ax[row,col].draw_artist(self.ax[row,col].patch)
                self.ax[row,col].draw_artist(lineList[index])
                index = index+1
        self.fig.canvas.update()
        self.fig.canvas.flush_events()

    
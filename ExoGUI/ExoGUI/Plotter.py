import multiprocessing as mp
import numpy as np
import matplotlib.pyplot as plt
import time


class Plotter(object):
    """description of class"""

    def __init__(self, tQue, yQue, lock, numY, dataLen, plotArray, titleArray, yLimArray, yLabelArray):
        self.numY = numY
        self.plotArray = plotArray
        self.titleArray = titleArray
        self.dataLen = dataLen
        self.yLimArray = yLimArray
        self.yLabelArray = yLabelArray

        self.numYPlot = np.int(np.sqrt(len(plotArray)))
        self.numXPlot = np.int(len(plotArray) / self.numYPlot)
        print(self.numYPlot)
        print(self.numXPlot)
        self.plotProcess = mp.Process(target=self.Plot, args=(tQue, yQue, lock))
        self.plotProcess.start()

    def Plot(self, tQue, yQue, lock):
        fig, axes = plt.subplots(self.numYPlot, self.numXPlot, figsize=(15, 10))
        tData = np.zeros(self.dataLen)
        yData = np.zeros((self.numY, self.dataLen))
        plt.show(block=False)
        lineList = []
        # fill t and y first
        for index in range(self.dataLen):
            while tQue.empty():
                time.sleep(0.001)
            with lock:
                tNow = tQue.get()
                yNow = yQue.get()
            tData[index] = tNow
            yData[:, index] = yNow.T
            print(index)
        tempIndex = 0
        for row in range(self.numYPlot):
            for col in range(self.numXPlot):
                line, = axes[row, col].plot(tData, yData[self.plotArray[tempIndex] - 1, :])
                lineList.append(line)
                fig.canvas.draw()
                axes[row, col].draw_artist(axes[row, col].patch)
                axes[row, col].draw_artist(lineList[tempIndex])
                axes[row, col].set_title(self.titleArray[tempIndex])
                axes[row, col].set_ylim(self.yLimArray[tempIndex])
                axes[row, col].set_xlabel('Time (sec)')
                axes[row, col].set_ylabel(self.yLabelArray[tempIndex])
                if self.refNeed[tempIndex]:
                    axes[row, col].axhline(y=self.refValue[tempIndex])
                tempIndex = tempIndex + 1
        while True:
            while yQue.empty():
                time.sleep(0.001)
            with lock:
                yNow = yQue.get()
            yData[:, :-1] = yData[:, 1:]
            yData[:, -1] = yNow
            tempIndex = 0
            for row in range(self.numYPlot):
                for col in range(self.numXPlot):
                    lineList[tempIndex].set_ydata(yData[self.plotArray[tempIndex] - 1, :])
                    axes[row, col].draw_artist(axes[row, col].patch)
                    axes[row, col].draw_artist(lineList[tempIndex])
                    tempIndex = tempIndex + 1
            fig.canvas.update()
            fig.canvas.flush_events()








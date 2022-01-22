import json
import argparse, sys
from ReadoutPB import ReadoutData
from PyQt5.QtWidgets import QApplication, QMainWindow, QMenu, QVBoxLayout, QSizePolicy, QMessageBox, QWidget, QPushButton
from matplotlib.backends.qt_compat import QtCore, QtWidgets
from matplotlib.backends.backend_qt5agg import (
        FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import mark_inset
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
import numpy as np
import time, threading

class App(QMainWindow):

    def __init__(self, readout):
        super().__init__()
        self.left = 10
        self.top = 10
        self.title = 'PyQt5 matplotlib example - pythonspot.com'
        self.width = 800
        self.height = 400
        self.initUI(readout)

    def initUI(self, readout):
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)

        self.m = Graph(self, readout)
        self.m.move(0,0)

        # button = QPushButton('PyQt5 button', self)
        # button.setToolTip('This s an example button')
        # button.move(500,0)
        # button.resize(140,100)
        self.show()
    def closeEvent(self, event):

        reply = QMessageBox.question(self, 'Message',
            "Are you sure to quit?", QMessageBox.Yes | 
            QMessageBox.No, QMessageBox.No)

        if reply == QMessageBox.Yes:
            self.m.readout.close()
            event.accept()
        else:
            event.ignore() 

class Graph(FigureCanvas):
    def __init__(self, parent, readout):
        self.fig, self.ax = plt.subplots(figsize=(10, 5))
        self.readout = readout
        FigureCanvas.__init__(self, self.fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                QSizePolicy.Expanding,
                QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)
        self.axins = inset_axes(self.ax, width="40%", height="30%",loc='lower left',
                    bbox_to_anchor=(0.2, 0.1, 1, 2),
                    bbox_transform=self.ax.transAxes)
        mark_inset(self.ax, self.axins, loc1=3, loc2=1, fc="none", ec='k', lw=1)
        self.ax.set_xlabel('t/ns')
        self.ax.set_ylabel('Amplitude/mV')
        self._timer = self.new_timer(2000)
        self._timer.add_callback(self.view)
        self._timer.start()
    def plot(self, waveforms, channelId):
        nch = len(channelId)
        waveforms = np.array(waveforms).reshape((nch,-1)).T
        self.ax.clear()
        self.axins.clear()
        self.ax.plot(waveforms, label=channelId)
        self.axins.plot(waveforms[:1000],label=['ch{}'.format(i) for i in channelId])
        self.axins.set_ylim([930,960])
        self.ax.legend()
        self.draw()
    def view(self):
        self.readout.SampleOne()
        self.plot(self.readout.readout.waveform, self.readout.readout.channelId)
    def thread(self, readout):
        self.thread = threading.Thread(target = self.view,args =(readout,))
        self.thread.start()
    def threadStop(self):
        self.thread.stop()

if __name__=='__main__':
    psr = argparse.ArgumentParser()
    psr.add_argument('-t', dest='type', default='extrigger')
    psr.add_argument('-s', dest='setting', default='config/setting.json')
    psr.add_argument('-c', dest='config', default='config/config.json')
    args = psr.parse_args()
    with open(args.setting, 'r') as ipt:
        deviceinfo = json.load(ipt)
    with open(args.config, 'r') as ipt:
        boardinfo = json.load(ipt)
    readout = ReadoutData(int(deviceinfo['address'][boardinfo['BoardId']], 16), boardinfo['BoardId'], boardinfo['sampleN'], boardinfo['postTriggerRatio'])
    if args.type=="trigger":
        readout.setTriggerMode(1, boardinfo['triggerch'])
    elif args.type=="pedestal":
        readout.setTriggerMode(0)
    elif args.type=="extrigger":
        readout.setTriggerMode(2, boardinfo['triggerch'], 1)
    samplech = boardinfo['samplech']
    readout.setSampleCh(samplech)
    readout.setPedestal("config/pedestal.txt")
    readout.setDevice()
    readout.SampleOne()
    app = QApplication(sys.argv)
    ex = App(readout)
    ex.showMaximized()
    # readout.close()
    sys.exit(app.exec_())
    # graph = Graph()
    # graph.plot(readout.readout.waveform,readout.readout.channelId)
    # graph.thread(readout)

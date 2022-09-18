import json
import argparse, sys
from ReadoutPB import ReadoutData
from PyQt5 import QtGui
from PyQt5.QtWidgets import QApplication, QMainWindow, QMenu, QVBoxLayout, QSizePolicy,QGridLayout, QAction, QMessageBox, QWidget, QPushButton, QLineEdit
from PyQt5.QtCore import QTimer
from matplotlib.backends.qt_compat import QtCore, QtWidgets
from matplotlib.backends.backend_qt5agg import (
        FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import mark_inset
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
import pyqtgraph as pg
import numpy as np
import time, threading

class App(QMainWindow):

    def __init__(self, readout, channelIds, triggerch=0):
        super().__init__()
        self.left = 10
        self.top = 10
        self.title = 'Sample Monitor'
        self.width = 800
        self.height = 400
        self.channelIds = channelIds
        self.triggerch = triggerch
        self.nch = len(self.channelIds)
        self.counter = 0
        self.counterL = 300
        self.initUI(readout)
        self.readout = readout
        self.initTimer()
    def initTimer(self):
        self._Figuretimer = QTimer()
        self._Figuretimer.timeout.connect(self.UpdateFigure)
        self._Sampletimer = QTimer()
        self._Sampletimer.timeout.connect(self.sample)
        self._Figuretimer.start(2000)
        self._Sampletimer.start(50)
    def initUI(self, readout):
        self.setWindowTitle(self.title)
        self.statusBar = self.statusBar()
        self.statusBar.showMessage('Ready')
        self.statusBar.show()
        self.show()

        # self.setGeometry(self.left, self.top, self.width, self.height)
        # layout = QtGui.QGridLayout()
        # w.setLayout(layout)
        # self.m = Graph(self, readout)
        # self.m.move(0,0)
        self.show()
        # 初始化 Menu， window的布局
        self.initMenu()
        self.initWindow()
    def initMenu(self):
        self.statusBar.showMessage('init menu')
        menubar = self.menuBar()
        fileMenu = menubar.addMenu('&File')
        sampleAct = QAction('Sample', self)
        sampleAct.triggered.connect(self.sample)
        fileMenu.addAction(sampleAct)

        HVsetAct = QAction('HVset', self)
        HVsetAct.triggered.connect(self.HVset)
        fileMenu.addAction(HVsetAct)

        recordMenu = menubar.addMenu('&Test')
        startAct = QAction('Start', self)
        startAct.triggered.connect(self.timerStart)
        recordMenu.addAction(startAct)
        stopAct = QAction('Stop', self)
        stopAct.triggered.connect(self.timerStop)
        recordMenu.addAction(stopAct)

        messageMenu = menubar.addMenu('&Analyze')
        analyzeAct = QAction('Analyze', self)
        analyzeAct.triggered.connect(self.analyze)
        messageMenu.addAction(analyzeAct)

        self.statusBar.showMessage('finish render menu')

    def initWindow(self):
        # 创建窗口主部件
        self.main_widget = QWidget()  
        # 创建主部件的网格布局
        self.main_layout = QGridLayout()  
        # 设置窗口主部件布局为网格布局
        self.main_widget.setLayout(self.main_layout)  

        # 创建左侧部件
        self.left_widget = QWidget()  
        self.left_widget.setObjectName('left_widget')
        # 创建左侧部件的网格布局层
        self.left_layout = QGridLayout()  
        # 设置左侧部件布局为网格
        self.left_widget.setLayout(self.left_layout) 

        # 创建右侧部件
        self.right_widget = QWidget() 
        self.right_widget.setObjectName('right_widget')
        self.right_layout = QGridLayout()
        self.right_widget.setLayout(self.right_layout) 

        # 左侧部件在第0行第0列，占20行2列
        self.main_layout.addWidget(self.left_widget, 0, 0, 20, 2) 
        # 右侧部件在第0行第6列，占20行10列
        self.main_layout.addWidget(self.right_widget, 0, 5, 20, 10)
        # 设置窗口主部件
        self.setCentralWidget(self.main_widget)

        # 初始化选项列表 leftlayout
        self.initStep()
        # 初始化图像列表, rightlayout
        self.initFigure()
        # 初始化监控面板
        self.initMonitor()
    def initFigure(self):
        # peak, charge, triggerate, control panel
        self.FigureUI = [[], [], [], []]
        self.FigureUI[0] = pg.PlotWidget()
        self.FigureUI[1] = pg.PlotWidget()
        self.FigureUI[2] = pg.PlotWidget()
        self.right_layout.addWidget(self.FigureUI[0], 0, 0, 1, 1)
        self.right_layout.addWidget(self.FigureUI[1], 0, 1, 1, 1)
        self.right_layout.addWidget(self.FigureUI[2], 1, 0, 1, 1)
        self.peakData = np.zeros((len(self.channelIds), 300))
        self.chargeData = np.zeros((len(self.channelIds), 300))
        self.triggerRateData = np.zeros((len(self.channelIds), 300))
        self.FigureCurve = [[], [], [], []]
        colors = [
            (0, 0, 255), (255, 0, 0), (255, 255, 255), (0, 255, 0), (100, 255, 0)
        ]
        print(len(self.channelIds))
        self.FigureUI[0].addLegend()
        self.FigureUI[1].addLegend()
        self.FigureUI[2].addLegend()
        for i in range(len(self.channelIds)):
            self.FigureCurve[0].append(self.FigureUI[0].plot(self.peakData[i], pen=pg.mkPen(color=colors[i]), name=self.channelIds[i]))
            self.FigureCurve[1].append(self.FigureUI[1].plot(self.chargeData[i], pen=pg.mkPen(color=colors[i]), name=self.channelIds[i]))
            self.FigureCurve[2].append(self.FigureUI[2].plot(self.triggerRateData[i], pen=pg.mkPen(color=colors[i]), name=self.channelIds[i]))
        self.FigureUI[0].setLabel("bottom", "peak")
        self.FigureUI[1].setLabel("bottom", "charge")
        self.FigureUI[2].setLabel("bottom", "Trigger Rate")
        

    def initStep(self):
        self.StepUI = [[], [], []]
        self.StepUI[0] = QLineEdit()
        self.StepUI[1] = QPushButton('start')
        self.StepUI[2] = QPushButton('end')
        for i in range(3):
            self.left_layout.addWidget(self.StepUI[i])
    def initMonitor(self):
        pass
    def UpdateFigure(self):
        self.triggerRateData[:, (self.counter+1)%self.counterL] = 0
        for i in range(len(self.channelIds)):
            self.FigureCurve[0][i].setData(self.peakData[i])
            self.FigureCurve[1][i].setData(self.chargeData[i])
            self.FigureCurve[2][i].setData(self.triggerRateData[i])
        self.counter = (self.counter+1)%self.counterL
        print('Update figure')
        self.StepUI[0].setText(str(self.counter))
    def sample(self):
        self.readout.SampleOne()
        peaks, charges = self.analyze(self.readout.readout.waveform)
        for i in range(self.nch):
            if peaks[i]>3 and peaks[i]<300:
                self.peakData[i, int(peaks[i])] += 1
            if charges[i]>25 and charges[i]<300:
                self.chargeData[i, int(charges[i])] += 1
            if peaks[i]>3 and charges[i]>25:
                self.triggerRateData[i, self.counter] += 1
    def HVset(self):
        pass
    def timerStart(self):
        pass
    def timerStop(self):
        pass
    def analyze(self, waveforms):
        waveforms = np.array(waveforms).reshape((self.nch,-1))
        peaks = np.zeros(self.nch)
        charges = np.zeros(self.nch)
        length = waveforms.shape[1]
        for i, c in enumerate(self.channelIds):
            if c == self.triggerch:
                continue
            w = waveforms[i]
            baseline = np.mean(w[:100])
            peaks[i] = baseline - np.min(w)
            pi = np.argmin(w)
            charges[i] = np.sum(baseline - w[max(pi-20,0):min(pi+75,length)])
        return peaks, charges
            
    def closeEvent(self, event):

        reply = QMessageBox.question(self, 'Message',
            "Are you sure to quit?", QMessageBox.Yes | 
            QMessageBox.No, QMessageBox.No)

        if reply == QMessageBox.Yes:
            self.readout.close()
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
        channelId = np.reshape(channelId, (-1))
        nch = len(channelId)
        waveforms = np.array(waveforms).reshape((nch,-1)).T
        self.ax.clear()
        self.axins.clear()
        self.ax.plot(waveforms, label=channelId)
        self.axins.plot(waveforms[:1000], label=['ch{}'.format(i) for i in channelId])
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
    ex = App(readout, samplech, boardinfo['triggerch'])
    ex.showMaximized()
    # readout.close()
    sys.exit(app.exec_())
    # graph = Graph()
    # graph.plot(readout.readout.waveform,readout.readout.channelId)
    # graph.thread(readout)

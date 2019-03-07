#!/usr/bin/env python3

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtChart import *
import sys

from connection import CubeConnection
from system import CubeSystemWidget


class CubeMainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.connection = CubeConnection()
        self.connection.stateChanged.connect(self.onConnectionStateChanged)
        self.connection.speedChanged.connect(self.onConnectionSpeedChanged)

        self.connect_tcp = QAction(QIcon.fromTheme('network-wired'), 'Connect to simulator')
        self.connect_tcp.triggered.connect(self.connection.connectViaTcp)
        self.connect_bt = QAction(QIcon.fromTheme('network-wireless'), 'Connect to cube')
        self.connect_bt.triggered.connect(self.connection.connectViaBluetooth)
        self.disconnect = QAction(QIcon.fromTheme('network-offline'), 'Disconnect')
        self.disconnect.triggered.connect(self.connection.disconnect)
        self.disconnect.setEnabled(False)

        toolbar = self.addToolBar('Connect')
        toolbar.setMovable(False)
        toolbar.setToolButtonStyle(Qt.ToolButtonTextBesideIcon)
        toolbar.addAction(self.connect_tcp)
        toolbar.addAction(self.connect_bt)
        toolbar.addAction(self.disconnect)

        self.speedLabel = QLabel('Not connected')
        self.speedLabel.setFrameStyle(QFrame.Panel | QFrame.Sunken);
        self.statusBar().setSizeGripEnabled(False)
        self.statusBar().addWidget(self.speedLabel, 1)

        self.systemWidget = CubeSystemWidget(self)
        self.addDockWidget(Qt.TopDockWidgetArea, self.systemWidget)
        placeholderLabel = QLabel('No app is running')
        placeholderLabel.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        self.setCentralWidget(placeholderLabel)

        self.setMinimumSize(800, 800)
        self.show()

    def onConnectionStateChanged(self, state):
        self.connect_tcp.setEnabled(state == CubeConnection.Disconnected)
        self.connect_bt.setEnabled(state == CubeConnection.Disconnected)
        self.disconnect.setEnabled(state == CubeConnection.Connected)
        if state != CubeConnection.Connected:
            self.speedLabel.setText('Not connected')
            placeholderLabel = QLabel('No app is running')
            placeholderLabel.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.setCentralWidget(placeholderLabel)
        else:
            self.readSpeedSeries = QLineSeries()
            self.readSpeedSeries.setName('Read')
            self.writeSpeedSeries = QLineSeries()
            self.writeSpeedSeries.setName('Write')
            self.speedCounter = 0
            self.speedChart = QChart()
            self.speedChart.addSeries(self.readSpeedSeries)
            self.speedChart.addSeries(self.writeSpeedSeries)
            self.speedChart.createDefaultAxes()
            self.speedChart.axisX().setRange(0,20)
            self.speedChart.axisY().setRange(0,20)
            self.speedChartView = QChartView(self.speedChart)
            self.setCentralWidget(self.speedChartView)

    def onConnectionSpeedChanged(self, read, write):
        self.speedLabel.setText('{:.1f} Bps read / {:.1f} Bps write'.format(read, write))
        self.readSpeedSeries.append(self.speedCounter, read)
        self.writeSpeedSeries.append(self.speedCounter, write)
        self.speedCounter += 1
        if self.speedCounter > 21:
            self.speedChart.scroll(self.speedChart.plotArea().width() / 20,0)
            self.readSpeedSeries.remove(0)
            self.writeSpeedSeries.remove(0)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    main_window = CubeMainWindow()
    app.exec_()

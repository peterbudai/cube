#!/usr/bin/env python3

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
import sys

from connection import CubeConnection
from system import CubeSystemWidget, CubeDynamicChart


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

        self.systemWidget = CubeSystemWidget(self)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.systemWidget)
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
            placeholderLabel = QLabel('No app is running')
            placeholderLabel.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.setCentralWidget(placeholderLabel)
            self.systemWidget.hide()
        else:
            self.speedChart = CubeDynamicChart('USART speed', 20)
            self.speedChart.addSeries('Read ({:.1f} Bps)', 20, Qt.green, Qt.AlignLeft)
            self.speedChart.addSeries('Write ({:.1f} Bps)', 20, Qt.red, Qt.AlignRight)
            self.setCentralWidget(self.speedChart)
            self.systemWidget.show()

    def onConnectionSpeedChanged(self, read, write):
        self.speedChart.appendSeries([read, write])


if __name__ == '__main__':
    app = QApplication(sys.argv)
    main_window = CubeMainWindow()
    app.exec_()

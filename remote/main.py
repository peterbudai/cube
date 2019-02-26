#!/usr/bin/env python3

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
import sys

from connection import CubeConnection


class CubeMainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.connection = CubeConnection()
        self.connection.stateChanged.connect(self.update_connect_state)

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

        self.show()

    def update_connect_state(self, state):
        self.connect_tcp.setEnabled(state == CubeConnection.Disconnected)
        self.connect_bt.setEnabled(state == CubeConnection.Disconnected)
        self.disconnect.setEnabled(state == CubeConnection.Connected)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    main_window = CubeMainWindow()
    app.exec_()

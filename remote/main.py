#!/usr/bin/env python3

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtNetwork import *
from PyQt5.QtBluetooth import *
import sys


class CubeConnection(QObject):
    Disconnected = 0
    Connecting = 1
    Connected = 2
    Disconnecting = 3

    stateChanged = pyqtSignal(int)

    def __init__(self, parent=None):
        super().__init__(parent)

    def prepareConnect(self, socket, host, address):
        self.progress = QProgressDialog('Connecting to {} ({})'.format(host, address.toString()), 'Cancel', 0, 0, self.parent())
        self.progress.setWindowTitle('Connecting')
        self.progress.setWindowModality(Qt.WindowModal)

        self.socket = socket
        self.socket.stateChanged.connect(self.onStateChanged)
        self.socket.error.connect(self.onConnectionError)

    def connectViaTcp(self):
        address = QHostAddress('127.0.0.1')
        self.prepareConnect(QTcpSocket(), 'localhost', address)
        self.socket.connectToHost(address, 28238)

    def connectViaBluetooth(self):
        address = QBluetoothAddress('F4:CB:52:66:5E:AD')
        self.prepareConnect(QBluetoothSocket(QBluetoothServiceInfo.RfcommProtocol), 'BUC-HUAWEI', address)
        self.socket.connectToService(address, QBluetoothUuid(QBluetoothUuid.SerialPort))

    def disconnect(self):
        self.socket.close()

    def onConnectionError(self, error):
        qDebug(self.socket.errorString())

    def onStateChanged(self, _):
        state = self.state()
        if state == CubeConnection.Connecting:
            self.progress.open(self.socket.close)
        else:
            self.progress.reset()
        self.stateChanged.emit(state)

    def state(self):
        if self.socket is None or self.socket.state() in {QAbstractSocket.UnconnectedState, QBluetoothSocket.UnconnectedState}:
            return CubeConnection.Disconnected
        elif self.socket.state() in {QAbstractSocket.ConnectedState, QBluetoothSocket.ConnectedState}:
            return CubeConnection.Connected
        elif self.socket.state() in {QAbstractSocket.ClosingState, QBluetoothSocket.ClosingState}:
            return CubeConnection.Disconnecting
        else:
            return CubeConnection.Connecting


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

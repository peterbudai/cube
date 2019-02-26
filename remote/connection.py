from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtNetwork import *
from PyQt5.QtBluetooth import *


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
        QMessageBox.critical(self.parent(), 'Connection error', self.socket.errorString())

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



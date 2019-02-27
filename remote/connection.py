from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtNetwork import *
from PyQt5.QtBluetooth import *

from crc8 import crc8

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
        self.socket.readyRead.connect(self.onRead)

        self.readBuffer = QByteArray()

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

    def onRead(self):
        self.readBuffer.append(self.socket.readAll())
        while not self.readBuffer.isEmpty():
            end = self.readBuffer.indexOf(b'\x7E')
            if end == -1:
                break;
            elif end == 0:
                self.readBuffer.remove(0, 1)
                continue
            else:
                frame = self.readBuffer.left(end)
                self.readBuffer.remove(0, end + 1)
                frameAddress = (ord(frame.at(0)) & 0x80) >> 7
                frameLength = ord(frame.at(0)) & 0x7F
                frame.replace(b'\x7D\x5E', b'\x7E')
                frame.replace(b'\x7D\x5D', b'\x7E')
                frameChecksum = crc8(frame.data()).digest()
                qDebug('{} {}: {}, {}, {}'.format(frame.toHex(), self.readBuffer.toHex(), frameAddress, frameLength, frameChecksum))

    def state(self):
        if self.socket is None or self.socket.state() in {QAbstractSocket.UnconnectedState, QBluetoothSocket.UnconnectedState}:
            return CubeConnection.Disconnected
        elif self.socket.state() in {QAbstractSocket.ConnectedState, QBluetoothSocket.ConnectedState}:
            return CubeConnection.Connected
        elif self.socket.state() in {QAbstractSocket.ClosingState, QBluetoothSocket.ClosingState}:
            return CubeConnection.Disconnecting
        else:
            return CubeConnection.Connecting



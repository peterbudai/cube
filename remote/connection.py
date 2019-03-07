import time

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtNetwork import *
from PyQt5.QtBluetooth import *

from crc8 import crc8


class CubeConnectionSpeed(QObject):
    updated = pyqtSignal(float, float)

    def __init__(self, secs, parent=None):
        super().__init__(parent)
        self.window = secs
        self.timer = QTimer(parent)
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.onTimeout)

    def start(self):
        self.read = [0 for i in range(self.window)]
        self.write = [0 for i in range(self.window)]
        self.index = 0
        self.timer.start()

    def stop(self):
        self.timer.stop()

    def onTimeout(self):
        readSpeed = sum(self.read) / self.window
        writeSpeed = sum(self.write) / self.window
        self.index = (self.index + 1) % self.window
        self.read[self.index] = 0
        self.write[self.index] = 0
        self.updated.emit(readSpeed, writeSpeed)

    def bytesRead(self, count):
        self.read[self.index] += count

    def bytesWritten(self, count):
        self.write[self.index] += count

class CubeConnection(QObject):
    Disconnected = 0
    Connecting = 1
    Connected = 2
    Disconnecting = 3

    stateChanged = pyqtSignal(int)
    speedChanged = pyqtSignal(float, float)

    System = 0
    Application = 1

    sysDataReceived = pyqtSignal()
    appDataReceived = pyqtSignal()
    sysDataSent = pyqtSignal()
    appDataSent = pyqtSignal()

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
        self.sysDataToRead = QByteArray()
        self.appDataToRead = QByteArray()
        self.writeBuffer = QByteArray()

        self.speed = CubeConnectionSpeed(10)
        self.speed.updated.connect(self.onSpeedChanged)

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

    def state(self):
        if self.socket is None or self.socket.state() in {QAbstractSocket.UnconnectedState, QBluetoothSocket.UnconnectedState}:
            return CubeConnection.Disconnected
        elif self.socket.state() in {QAbstractSocket.ConnectedState, QBluetoothSocket.ConnectedState}:
            return CubeConnection.Connected
        elif self.socket.state() in {QAbstractSocket.ClosingState, QBluetoothSocket.ClosingState}:
            return CubeConnection.Disconnecting
        else:
            return CubeConnection.Connecting

    def onConnectionError(self, error):
        QMessageBox.critical(self.parent(), 'Connection error', self.socket.errorString())

    def onStateChanged(self, _):
        state = self.state()
        if state == CubeConnection.Connecting:
            self.progress.open(self.socket.close)
        else:
            self.progress.reset()
        if state == CubeConnection.Connected:
            self.speed.start()
        else:
            self.speed.stop()
        self.stateChanged.emit(state)

    def onSpeedChanged(self, read, write):
        self.speedChanged.emit(read, write)

    def onRead(self):
        self.speed.bytesRead(self.socket.bytesAvailable())

        received = set()
        self.readBuffer.append(self.socket.readAll())
        while not self.readBuffer.isEmpty():
            end = self.readBuffer.indexOf(b'\x7E')
            if end == -1:
                break
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
                frameChecksum = crc8(frame.data()).digest()[0]
                if frame.size() != frameLength and frameChecksum != 0:
                    qDebug('Frame {}: addr {}, len {}, crc {} not ok'.format(frame.toHex(), frameAddress, frameLength, frameChecksum))
                    continue
                if frameAddress == CubeConnection.System:
                    self.sysDataToRead += frame.mid(1, frameLength)
                    qDebug('Sys frame {}: len {}, crc {} ok, buf {}'.format(frame.toHex(), frameLength, frameChecksum, len(self.sysDataToRead)))
                    received.add(CubeConnection.System)
                else:
                    self.appDataToRead += frame.mid(1, frameLength)
                    qDebug('App frame {}: len {}, crc {} ok, buf {}'.format(frame.toHex(), frameLength, frameChecksum, len(self.appDataToRead)))
                    received.add(CubeConnection.Application)

        if CubeConnection.System in received:
            self.sysDataReceived.emit()
        if CubeConnection.Application in received:
            self.appDataReceived.emit()


#!/usr/bin/env python

from crc8 import crc8
from socket import socket, AF_INET, SOCK_STREAM
from sys import argv

def wrap_message(data):
    msg = chr(128 + len(data)) + data
    msg = msg + crc8(msg).digest()
    msg = msg.replace(chr(0x7E), '\x7D\x5E').replace(chr(0x7D), '\x7D\x5D')
    msg = chr(0x7E) + msg + chr(0x7E)
    return msg

def send_message(msg):
    sock = socket(AF_INET, SOCK_STREAM)
    sock.connect(('127.0.0.1', 28238))
    sock.send(msg)
    sock.close()

def unwrap_message(msg):
    s = msg.find(chr(0x7E),0)
    if s < 0:
        return msg
    e = msg.find(chr(0x7E),s+1)
    if e < 0:
        return msg[s:]
    if e - s < 3:
        return msg[e:]
    data = msg[s+1:e].replace('\x7D\x5E', chr(0x7E)).replace('\x7D\x5D', chr(0x7D))
    if (ord(data[0]) - 128 != len(data) - 2) or (data[-1] != crc8(data[:-1]).digest()):
        print(ord(data[0]), len(data), ':'.join('{:02x}'.format(ord(c)) for c in data))
    else:
        print('\n'.join('{:08b}'.format(ord(c))[::-1] for c in data[1:-1]))
    print('')
    return msg[e:]

def recv_message():
    sock = socket(AF_INET, SOCK_STREAM)
    sock.connect(('127.0.0.1', 28238))
    msg = b''
    while True:
        msg = unwrap_message(msg + sock.recv(64))
    sock.close()
    return msg


if __name__ == '__main__':
    if argv[1] == 'send':
        send_message(wrap_message(argv[2][:127] if len(argv) > 1 else ''))
    elif argv[1] == 'recv':
        recv_message()

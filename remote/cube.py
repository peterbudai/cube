#!/usr/bin/env python

from crc8 import crc8
from socket import socket, AF_INET, SOCK_STREAM
from sys import argv

def wrap_message(data):
    msg = chr(128 + len(data)) + data
    msg = msg + crc8(msg).digest()
    msg = msg.replace(chr(0x7E), '\x7D\x5E')
    msg = chr(0x7E) + msg + chr(0x7E)
    return msg

def send_message(msg):
    sock = socket(AF_INET, SOCK_STREAM)
    sock.connect(('127.0.0.1', 28238))
    sock.send(msg)
    sock.close()

if __name__ == '__main__':
    send_message(wrap_message(argv[1][:127] if len(argv) > 1 else ''))

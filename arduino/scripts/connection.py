import serial
import abc
from typing import override
import socket


class Connection:
    @abc.abstractmethod
    def write(self, data: bytearray):
        ...

    @abc.abstractmethod
    def read(self) -> bytearray:
        ...


class SerialConnection(Connection):
    def __init__(self, serial_port, baud_rate):
        self.ser = serial.Serial(serial_port, baud_rate, timeout=1)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.ser.close()

    @override
    def write(self, data):
        self.ser.write(data)

    @override
    def read(self):
        return self.ser.readline()


class TcpConnection(Connection):
    def __init__(self, address, port=23):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(1)
        self.sock.connect((address, port))

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.sock.close()

    @override
    def write(self, data):
        print(str(data))
        self.sock.sendall(data)

    @override
    def read(self):
        return self.readline(3)

    def readline(self, retries_left):
        '''Receive a response line ignoring spurious responses like \r\n.'''
        if not retries_left:
            return b''
        try:
            data = self.sock.recv(1024)
            if not data.strip():
                return self.readline(retries_left - 1)
            return data
        except TimeoutError:
            return b''

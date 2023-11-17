import pickle
import socket 
import os
import struct


comm_path = 'some-default-path'
client_socket: socket.socket


def connect() -> int:
    global comm_path
    global client_socket
    with open('db2_config', 'r') as config:
        comm_path = config.readline()

    client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    client_socket.connect(comm_path)

    status_buf = client_socket.recv(4)

    status = struct.unpack('i', status_buf)[0]

    return status
    

def insert(key: str, value) -> int:
    svalue = pickle.dumps(value)

    op = struct.pack('iIII', 0, 16 + len(key) + len(svalue), len(key), len(svalue))
    op += key.encode()
    op += svalue

    client_socket.sendall(op)


    status_buf = client_socket.recv(4)

    status = struct.unpack('i', status_buf)[0]

    return status

def find(key: str):
    op = struct.pack('iIII', 1, 16 + len(key), len(key), 0)
    op += key.encode()

    client_socket.sendall(op)

    status_buf = client_socket.recv(4)
    print(f'response size for find {len(status_buf)}')

    status = struct.unpack('i', status_buf)[0]

    response_buf = client_socket.recv(0x4000)

    if (status == 200):
        return pickle.loads(response_buf)
    
    return None
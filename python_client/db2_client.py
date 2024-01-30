import pickle
import socket 
import struct

def simple_hash(key: str) -> int:
    key_len = len(key)
    h = 5381

    for i in range(key_len):
        h = (h * 33) + ord(key[i])

    return h & 0xffffffffffffffff


class Client:
    comm_path = '/home/dekel/.db2/db2_comm'
    client_socket: socket.socket

    def __init__(self) -> None:
        print('client created!')

    def connect(self) -> int:
        self.client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

        self.client_socket.connect(self.comm_path)

        status_buf = self.client_socket.recv(4)

        status = struct.unpack('i', status_buf)[0]

        return status
        

    def insert(self, key: str, value) -> int:
        svalue = pickle.dumps(value)
        key_hash = simple_hash(key)

        print('insert op key size {} val size {} key hash {}'.format(len(key), len(svalue), key_hash))
        x = len(key)
        op = struct.pack('iIIQ', 0, x, len(svalue), key_hash)
        print('insert op', op, len(op))

        self.client_socket.sendall(op)


        status_buf = self.client_socket.recv(4)
        status = struct.unpack('i', status_buf)[0]


        if status == 200:
            self.client_socket.sendall(key)
            self.client_socket.sendall(svalue)
            status_buf = self.client_socket.recv(4)
            status = struct.unpack('i', status_buf)[0]

        return status

    def find(self, key: str):
        op = struct.pack('iQ', 1, simple_hash(key))

        self.client_socket.sendall(op)

        status_buf = self.client_socket.recv(8)
        print(f'response size for find {len(status_buf)}')

        status, value_size = struct.unpack('ii', status_buf)

        if (status == 200):
            print('will recv a value of {} bytes'.format(value_size))
            response_buf = self.client_socket.recv(value_size)
            return pickle.loads(response_buf)
        
        return None
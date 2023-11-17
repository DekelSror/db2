
from functools import reduce
import string
from db2_client import connect, insert, find
from random import randint, choice

class UserData:
    a: int
    b: int
    c: int
    d: int

    def __init__(self,a: int, b: int, c: int, d: int) -> None:
        self.a = a
        self.b = b
        self.c = c
        self.d = d


def udJsonEncode(ud: UserData):
    return {'a': ud.a, 'b': ud.b, 'c': ud.c, 'd': ud.d}

def generate_ud():
    return UserData(randint(0, 100), randint(0, 100), randint(0, 100), randint(0, 100))

def generate_key():
    return reduce(lambda s, elem: s + elem, [choice(string.ascii_letters) for _ in range(0, randint(6, 16))], '')

def repr_ud(ud: UserData) -> str:
    return f'ud a={ud.a} b={ud.b} c={ud.c} d={ud.d}'

status_connect = connect()

test_key = generate_key()
test_value = generate_ud()

print(f'insert data - key {test_key}\n value {repr_ud(test_value)}')

status_insert = insert(test_key, test_value)
print(f'connect {status_connect} insert {status_insert}')

found: UserData = find(test_key)

print('found ' + (repr_ud(found) if found != None else 'None'))






import * as net from 'net'
import * as fs from 'fs'
import { promisify } from 'util'


enum Op { insert, find, remove } 

type db2_response = {
    status: number
    body: string
}


type db_op = {
    op: Op
    size: number
    header: {
        key_size: number
        value_size: number
    }    
    data: string
}

const db2_socket_path = '/home/dekel/src/db2/db2_comm'

const parse_response: (raw: Buffer) => db2_response = raw => {
    return {
        status: raw.readUint32LE(0),
        body: raw.toString('utf-8', 4)
    }
}

const bufferize_op = (op: db_op): Buffer => {
    const bop = Buffer.alloc(op.size)
    
    bop.writeInt32LE(op.op, 0)
    bop.writeUInt32LE(op.size, 4)
    bop.writeUInt32LE(op.header.key_size, 8)
    bop.writeUInt32LE(op.header.value_size, 12)
    bop.write(op.data, 16)

    return bop
}

class Db2Client {
    private socket: net.Socket

    constructor() {
        this.socket = net.createConnection({path: db2_socket_path}, () => {
            console.log('node client connected!')
        })
    }

    insert = <T>(key: string, value: T): Promise<number> => {
        const rv = new Promise<number>((resolve, reject) => {
            this.socket.once('data', data => {
                const res = parse_response(data)
    
                resolve(res.status)
            })
        })
        
        const strd = JSON.stringify(value)

        const op: db_op = {
            op: Op.insert,
            size: 16 + key.length + strd.length,
            header: {
                key_size: key.length,
                value_size: strd.length
            },
            data: key + strd
        }
    
        this.socket.write(bufferize_op(op), console.log)

        return rv
    }

    find = <T>(key: string): Promise<T> => {
        const rv = new Promise<T>((resolve, reject) => {
            this.socket.once('data', data => {
                const res = parse_response(data)
    
                if (res.status === 200) {
                    resolve(JSON.parse(res.body) as T)       
                } else {
                    resolve(undefined)
                }
            })
        })

        const op: db_op = {
            op: Op.find,
            size: 16 + key.length,
            header: {
                key_size: key.length,
                value_size: 0
            },
            data: key
        }
    
        this.socket.write(bufferize_op(op))

        return rv
    }
}


const Db2 = new Db2Client()

export default Db2

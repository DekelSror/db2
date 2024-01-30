import * as net from 'net'
import * as Db2Config from './db2_config.json'
import { open, readSync, writeSync } from 'fs';

const simple_hash = (key: string): bigint => {
    const len = key.length;
    let h = 5381;

    for (let i = 0; i < len; i++)
    {
        h = (h * 33) + Number.parseInt(key[i].toLowerCase(), 36) - 10;
    }

    return BigInt(h);
}

enum Op { kv_insert, kv_find, kv_remove, ts_create, ts_add, ts_get_range, ts_start_end } 

let socket: net.Socket

const db2_connect = () => {
    console.log(Db2Config.db2_comm_path)
    socket = net.createConnection({path: Db2Config.db2_comm_path}, () => console.log('connected to db2'))

    return Boolean(socket) ? 200 : 500
}



const db2_kv_insert = <T>(key: string, value: T): Promise<number> => {
    const strd = JSON.stringify(value)
    const key_hash = simple_hash(key)   
    console.log('kv_insert key_size (+1)', key.length + 1, 'val_size', strd.length, 'hash', key_hash.toString(10))

    // send op
    // revc response
    // on 200
    //  send value
    //  recv repsonse - return status
    // on !200
    //  return status

    const rv = new Promise<number>((resolve, reject) => {
        socket.once('data', data => {
            const status = data.readInt32LE(0)

            if (status === 200)
            {
                socket.write(key + '\0', err => console.log(err ?? 'streamed key'))
                socket.write(strd, err => console.log(err ?? 'streamed value'))

                socket.once('data', response => console.log('second insert response', response))
            }
            
            resolve(status)
        })
    })

    const buf = Buffer.alloc(32)
    buf.fill(0)

    buf.writeInt32LE(Op.kv_insert, 0)
    buf.writeUInt32LE(key.length + 1, 8)
    buf.writeUInt32LE(strd.length, 12)

    buf.writeBigUint64LE(key_hash, 16)

    
    socket.write(buf, err => console.error(err ?? 'insert op sent'))

    return rv
}

const db2_kv_find = <T>(key: string): Promise<T> => {

    // send op
    // recv response
    // on 200
    //  recv value - return it
    
    let response = {
        status: 500,
        value_size: 0
    }

    let rv = new Promise<T>((resolve, reject) => {
        socket.once('data', data => {
            response.status = data.readInt32LE(0),
            response.value_size = data.readUInt32LE(4)
        })
        
        if (response.status === 200) {
            console.log('found! streaming in the value')
            const val_buf: Buffer = socket.read(response.value_size)


            resolve(JSON.parse(val_buf.toString()) as T)
        }
        else {
            console.log('no find')
            resolve(undefined)
        }
    })

    const buf = Buffer.alloc(32).fill(0)
    buf.writeInt32LE(Op.kv_find, 0)
    buf.writeBigInt64LE(simple_hash(key), 8)

    socket.write(buf, err => console.log(err ?? 'sent find op'))    

    return rv
}


export {db2_connect, db2_kv_insert, db2_kv_find}


// import the client

import Db2 from "./db2_client";

type UserData = {
    a: number
    b: number
    c: number
    d: number
}


const generate_ud = (): UserData => {
    return {
        a: Math.floor(Math.random() * 100),
        b: Math.floor(Math.random() * 100),
        c: Math.floor(Math.random() * 100),
        d: Math.floor(Math.random() * 100),
    }
}

const generate_key = (): string => {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
    const len = Math.floor(Math.random() * 11) + 6
    
    let rv :string[] = new Array<string>(len).fill('x')
    
    console.log(len)
    console.log(rv)
    for (let i = 0; i < len; i++) {
        rv[i] =  chars[Math.floor(Math.random() * chars.length)]
    }

    return rv.join('')
}

// first two args are node and driver2.ts
const test_size = +process.argv[2]

const insert_find_test = async() => {
    console.log('insert_find_test with test_size', test_size)
    const values = new Array(test_size).fill(0).map(() => generate_ud())
    const keys = new Array(test_size).fill(0).map(() => generate_key())
    
    for (let i = 0; i < test_size; i++) {
        const status = await Db2.insert(keys[i], values[i])
        
        if (status !== 200) {
            console.log('insert error', status, 'with key', keys[i])
        }
    }
    
    for (let i = 0; i < test_size; i++) {
        const val: UserData = await Db2.find(keys[i])

        if (val) {
            console.log('find equal to thing?', JSON.stringify(val) === JSON.stringify(values[i]))
        } else {
            console.log('find error for key', keys[i])
        }
    }
}

// insert_find_test().then(() => console.log('driver done'))

const single_insert_find = (): void => {
    const ud = generate_ud()
    console.log('insert find one test with', ud)
    Db2.insert("smallak", ud).then(status => {
        if (status === 200) {
            console.log('insert good!')
            Db2.find<UserData>("smallak").then(console.log)
    
        } else {
            console.log('insert problem', status)
        }
    })
}

Db2.connect()
insert_find_test().then(() => console.log('done'))
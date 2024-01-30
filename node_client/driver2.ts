import { db2_kv_insert, db2_connect, db2_kv_find } from './db2_client';



console.log('connect result', db2_connect())

type Test = {yes: number}



const test = async() => {
    const insert_res = await db2_kv_insert<Test>('key1', {yes: 111})
    
    if (insert_res === 200) {
        const find_res = await db2_kv_find<Test>('key1')
        
        console.log(find_res)
    }
}


test().then(() => console.log('client done'))



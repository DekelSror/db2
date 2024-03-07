#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "utilities.h"
#include "db2_mempool.h"
#include "db2_kv.h"

static db_entry_t db[db2_num_entries] = {0};
static uint32_t db_size = 0;

static long hash_index(uint64_t hash);


int kv_can_insert(struct db_op_insert_t header)
{
    uint32_t required_memory = header._key_size + header._val_size + sizeof(db_value_t) * 2;
    return Mempool.has(required_memory) && db_size < db2_num_entries;
}


int kv_insert(uint64_t key_hash, db_value_t* key, db_value_t* val)
{
    long index = key_hash % db2_num_entries;
    const long init_index = index;

    do
    {
        if (db[index]._hash == key_hash)
        {
            // this is also not the purest
            Mempool.free(db[index]._key);
            Mempool.free(db[index]._val);
            db[index]._hash = 0;
        }

        if (db[index]._hash == 0)
        {
            db[index]._hash = key_hash;
            db[index]._val = val;
            db[index]._key = key;

            db_size++;

            return 0;
        }

        index++;
        if (index == db2_num_entries)
        {
            index = 0;
        }
    } while (index != init_index);

    // this should never execute
    return 1;
}

int kv_remove(struct db_op_remove_t header)
{
    long index = hash_index(header._key_hash);

    if (index >= 0)
    {
        db[index]._hash = 0;
        Mempool.free(db[index]._key);
        Mempool.free(db[index]._val);
        db[index]._key = NULL;
        db[index]._val = NULL;

        db_size--;
    }
    
    return index < 0; // for positive 0
}

db_value_t* kv_find(struct db_op_find_t header)
{
    long index = hash_index(header._key_hash);

    if (index < 0)
    {
        return NULL;
    }

    return db[index]._val;
}


//

static long hash_index(uint64_t hash)
{
    long index = hash % db2_num_entries;
    const long init_index = index;

    do
    {
        if (db[index]._hash == hash)
        {
            return index;
        }

        index++;
        if (index == db2_num_entries)
        {
            index = 0;
        }
    } while (index != init_index);

    return -1;
}
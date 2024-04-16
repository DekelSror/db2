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
    uint32_t required_memory = header.key_size + header.val_size + sizeof(db2_value_t) * 2;
    return Mempool.has(required_memory) && db_size < db2_num_entries;
}


int kv_insert(uint64_t key_hash, db2_value_t* key, db2_value_t* val)
{
    long index = key_hash % db2_num_entries;
    const long init_index = index;

    do
    {
        if (db[index].hash == key_hash)
        {
            // this is also not the purest
            Mempool.free(db[index].key);
            Mempool.free(db[index].val);
            db[index].hash = 0;
        }

        if (db[index].hash == 0)
        {
            db[index].hash = key_hash;
            db[index].val = val;
            db[index].key = key;

            outl("kv insert at index %ld", index);

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
    long index = hash_index(header.key_hash);

    if (index >= 0)
    {
        db[index].hash = 0;
        Mempool.free(db[index].key);
        Mempool.free(db[index].val);
        db[index].key = NULL;
        db[index].val = NULL;

        db_size--;
    }
    
    return index < 0; // for positive 0
}

db2_value_t* kv_find(struct db_op_find_t header)
{
    long index = hash_index(header.key_hash);

    outl("kv find index %ld", index);

    if (index < 0)
    {
        return NULL;
    }

    return db[index].val;
}


//

static long hash_index(uint64_t hash)
{
    long index = hash % db2_num_entries;
    const long init_index = index;

    do
    {
        if (db[index].hash == hash)
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
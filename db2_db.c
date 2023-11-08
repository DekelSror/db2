#include <stdio.h>
#include <string.h>

#include "db2_mempool.h"
#include "db2_types.h"
#include "db2.h"

static db_entry db[db2_num_entries] = { 0 };

static long entry_index(char* key, unsigned key_size);
static uint64_t simple_hash(char* key, uint32_t len);
uint64_t(*db_hash)(char*, uint32_t) = simple_hash;

int handle_insert(db_op_t* op)
{
    db_op_insert_t header = op->_header._insert;
    outl("insert key_size %u val_size %u", header._key_size, header._val_size);
    
    db_value_t* key_block = (db_value_t*)Mempool.allocate(header._key_size + 4);
    key_block->_size = header._key_size;

    db_value_t* val_block = (db_value_t*)Mempool.allocate(header._val_size + 4); // 4 for size
    val_block->_size = header._val_size;

    memmove(key_block->_val, op->_body, header._key_size);
    memmove(val_block->_val, op->_body + header._key_size, header._val_size);

    uint64_t key_hash = db_hash(key_block->_val, key_block->_size);

    long index = key_hash % db2_num_entries;
    const long init_index = index;
    outl("handle_insert init index for key '%s' is %ld hash is %lu", key_block->_val, init_index, key_hash);

    do 
    {
        if (db[index]._hash == key_hash)
        {
            Mempool.free(db[index]._key);
            Mempool.free(db[index]._val);
            db[index]._hash = 0;
        }

        if (db[index]._hash == 0)
        {
            db[index]._hash = key_hash;
            db[index]._val = val_block;
            db[index]._key = key_block;

            outl("handle_insert inserted key '%s' at index %ld", key_block->_val, index);

            return 0;
        }

        index++;
        if (index == db2_num_entries)
        {
            index = 0;
        }
    } while (index != init_index);

    outl("could not find an index for '%s'", key_block->_val);

    return 1;
}

int handle_remove(db_op_t* op)
{
    db_op_remove_t header = op->_header._remove;
    
    long index = entry_index(op->_body, header._key_size);

    if (index >= 0)
    {
        Mempool.free(db[index]._val);
        db[index]._hash = 0;
        db[index]._val = 0;
        db[index]._key = 0;
    }

    return 0;
}

db_value_t* handle_find(db_op_t* op)
{
    db_op_find_t header = op->_header._find;

    long index = entry_index(op->_body, header._key_size);

    if (index >= 0)
    {
        return db[index]._val;
    }
    
    return NULL;
}

//

static long entry_index(char* key, unsigned key_size)
{
    uint64_t hash = db_hash(key, key_size);

    long index = hash % db2_num_entries;

    const long init_index = index;

    outl("entry_index init index for key '%s' %ld hash is %lu ", key, index, hash);

    do
    {
        if (db[index]._hash == hash)
        {
            outl("entry_index found index %ld for key '%s'", index, key);
            return index;
        }

        index++;
        if (index == db2_num_entries) 
        {
            index = 0;
        }
    } while (index != init_index);

    outl("entry_index not found for key '%s'", key);

    return -1;
}

static uint64_t simple_hash(char* key, uint32_t _len)
{
    int len = (int)_len;
    uint64_t h = 5381;

    for (int i = 0; i < len; i++)
    {
        h = (h * 33) + key[i];
    }

    return h;
}
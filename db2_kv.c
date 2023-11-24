#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "utilities.h"
#include "db2_mempool.h"
#include "db2_types.h"
#include "db2.h"

static db_entry_t db[db2_num_entries] = {0};

static long entry_index(char *key, unsigned key_size);
static long hash_index(uint64_t hash);
static uint64_t simple_hash(char *key, uint32_t len);
uint64_t (*db_hash)(char *, uint32_t) = simple_hash;

int handle_insert(db_op_t *op, int client_socket)
{
    struct db_op_insert_t header = op->_header._insert;
    outl("insert key_size %u val_size %u", header._key_size, header._val_size);

    db_value_t *key_block = (db_value_t *)Mempool.allocate(header._key_size + sizeof(db_value_t));
    key_block->_size = header._key_size;

    db_value_t *val_block = (db_value_t *)Mempool.allocate(header._val_size + sizeof(db_value_t));
    val_block->_size = header._val_size;

    db_response_t response = {._status = 200};
    ssize_t ack_sent = send(client_socket, &response, sizeof(db_response_t), 0);

    stream_in(client_socket, key_block->_val, key_block->_size);
    stream_in(client_socket, val_block->_val, val_block->_size);

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

    outl("handle_insert could not find an index for '%s'", key_block->_val);
    response._status = 404;

    return 1;
}

int handle_remove(db_op_t *op)
{
    struct db_op_remove_t header = op->_header._remove;

    long index = hash_index(header._key_hash);

    if (index >= 0)
    {
        db[index]._hash = 0;
        Mempool.free(db[index]._key);
        Mempool.free(db[index]._val);
        db[index]._key = NULL;
        db[index]._val = NULL;
    }

    return 0;
}

int handle_find(db_op_t *op, int client_socket)
{
    struct db_op_find_t header = op->_header._find;

    long index = hash_index(header._key_hash);

    if (index >= 0)
    {
        db_response_t response = { ._status = 200, ._body_size = db[index]._val->_size };
        send(client_socket, &response, sizeof(db_response_t), 0);
        stream_out(client_socket, db[index]._val->_val, db[index]._val->_size);
        return 0;
    }
    else
    {
        db_response_t response = { ._status = 404 };
        send(client_socket, &response, sizeof(db_response_t), 0);
    }

    return 1;
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

static long entry_index(char *key, unsigned key_size)
{
    return hash_index(db_hash(key, key_size));
}

static uint64_t simple_hash(char *key, uint32_t _len)
{
    int len = (int)_len;
    uint64_t h = 5381;

    for (int i = 0; i < len; i++)
    {
        h = (h * 33) + key[i];
    }

    return h;
}
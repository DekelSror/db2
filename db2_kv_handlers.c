#include "db2_mempool.h"
#include "utilities.h"
#include "db2_kv.h"
 
int handle_insert(db_op_t *op, int client_socket)
{
    db_response_t response = {._status = 200};
    struct db_op_insert_t header = op->_header._insert;

    outl("kv_insert key_size %u val_size %u hash %lu", header._key_size, header._val_size, header._key_hash);

    if (!kv_can_insert(header))
    {
        response._status = 500;
        send_response(client_socket, &response);
        return 1;
    }

    db_value_t *key_block = (db_value_t*)Mempool.allocate(header._key_size + sizeof(db_value_t));
    key_block->_size = header._key_size;

    db_value_t *val_block = (db_value_t*)Mempool.allocate(header._val_size + sizeof(db_value_t));
    val_block->_size = header._val_size;

    send_response(client_socket, &response);

    stream_in(client_socket, key_block->_val, key_block->_size);
    stream_in(client_socket, val_block->_val, val_block->_size);
    outl("got kv from client");

    return kv_insert(header._key_hash, key_block, val_block);
}

int handle_remove(db_op_t *op, int client_socket)
{
    db_response_t response = {._status = 200};

    int removed = kv_remove(op->_header._remove);

    if (!removed)
    {
        response._status = 500;
    }
    
    send_response(client_socket, &response);

    return 0;
}

int handle_find(db_op_t *op, int client_socket)
{
    db_response_t response = {._status = 200, ._body_size = 0};

    db_value_t* found = kv_find(op->_header._find);

    if (found != NULL)
    {
        db_response_t response = { ._status = 200, ._body_size = found->_size };
        send_response(client_socket, &response);
        stream_out(client_socket, found->_val, found->_size);
    }
    else
    {
        response._status = 404;
        send_response(client_socket, &response);
    }

    return response._status != 200;
}
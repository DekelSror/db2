#include "db2_mempool.h"
#include "utilities.h"
#include "db2_kv.h"
 
int handle_insert(db_op_t *op, int client_socket)
{
    db_response_t response = {.status = 200};
    struct db_op_insert_t header = op->header.insert;

    if (kv_can_insert(header))
    {
        send_response(client_socket, &response);
        db2_value_t *key_block = (db2_value_t*)Mempool.allocate(header.key_size + sizeof(db2_value_t));
        key_block->size = header.key_size;

        db2_value_t *val_block = (db2_value_t*)Mempool.allocate(header.val_size + sizeof(db2_value_t));
        val_block->size = header.val_size;


        stream_in(client_socket, key_block->val, key_block->size);
        stream_in(client_socket, val_block->val, val_block->size);
        send_response(client_socket, &response);

        return kv_insert(header.key_hash, key_block, val_block);
    }

    response.status = 500;
    send_response(client_socket, &response);
    return 1;
}

int handle_remove(db_op_t *op, int client_socket)
{
    db_response_t response = {.status = 200};

    int removed = kv_remove(op->header.remove);

    if (!removed)
    {
        response.status = 500;
    }
    
    send_response(client_socket, &response);

    return 0;
}

int handle_find(db_op_t *op, int client_socket)
{
    db_response_t response = {.status = 200, .body_size = 0};

    db2_value_t* found = kv_find(op->header.find);

    if (found != NULL)
    {
        db_response_t response = { .status = 200, .body_size = found->size };
        send_response(client_socket, &response);
        stream_out(client_socket, found->val, found->size);
    }
    else
    {
        response.status = 404;
        send_response(client_socket, &response);
    }

    return response.status != 200;
}
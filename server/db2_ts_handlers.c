#include <sys/socket.h>

#include "db2_mempool.h"
#include "db2_timeseries.h"
#include "utilities.h" 
#include "db2_ts_handlers.h"

int handle_create(db_op_t* op, int client_socket)
{
    db_response_t response = { .status = 200 };
    struct db_op_ts_create_t header = op->header.ts_create;

    if (!timeseries_can_create(header))
    {
        response.status = 500;
        outl("already have %d timeseries", db2_max_timeseries);
        send_response(client_socket, &response);
        return 1;
    }

    db2_value_t* key_block = Mempool.allocate(header.key_size + sizeof(db2_value_t));
    key_block->size = header.key_size;

    send_response(client_socket, &response);
    stream_in(client_socket, key_block->val, header.key_size);

    int res = timeseries_create(key_block);

    response.body_size = res;
    send_response(client_socket, &response);


    return 0;
}

int handle_timeseries_add(db_op_t* op, int client_socket)
{
    db_response_t response = { .status = 200 };
    struct db_op_ts_add_t header = op->header.ts_add;
    
    int added = timeseries_add(header);

    if (!added)
    {
        response.status = 500;
    }

    send_response(client_socket, &response);

    return !(response.status == 200);
}
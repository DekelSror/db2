#include <sys/socket.h>

#include "db2_mempool.h"
#include "db2_timeseries.h"
#include "utilities.h" 
#include "db2_ts_handlers.h"

int handle_create(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };
    struct db_op_ts_create_t header = op->_header._ts_create;

    if (!timeseries_can_create(header))
    {
        response._status = 500;
        outl("already have %d timeseries", db2_max_timeseries);
        send_response(client_socket, &response);
        return 1;
    }

    db_value_t* key_block = Mempool.allocate(header._key_size + sizeof(db_value_t));
    key_block->_size = header._key_size;

    send_response(client_socket, &response);
    stream_in(client_socket, key_block->_val, header._key_size);

    int res = timeseries_create(key_block);

    response._body_size = res;
    send_response(client_socket, &response);


    return 0;
}

int handle_timeseries_add(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };
    struct db_op_ts_add_t header = op->_header._ts_add;
    
    int added = timeseries_add(header);

    if (!added)
    {
        response._status = 500;
    }

    send_response(client_socket, &response);

    return !(response._status == 200);
}


int handle_ts_start_end(db_op_t* op, int client_socket)
{
    struct db_op_ts_start_end_t header = op->_header._ts_start_end;

    struct {
        int _status;
        db2_time_t _time;
    } response = {
        ._status = 200,
        ._time =  timeseries_start_end(header)
    };
    
    send(client_socket, &response, sizeof(response), 0);

    return 200;
}


int handle_timeseries_get_range(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };

    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    struct ts_slice_t slice = timeseries_get_range(header);

    if (slice.count == 0 || slice.start == NULL)
    {
        response._status = 404;
        send_response(client_socket, &response);

        return 1;
    }

    response._body_size = (slice.count + 1) * sizeof(timeseries_entry_t);
    send_response(client_socket, &response);
    
    stream_out(client_socket, (const char*)slice.start, response._body_size);
    
    send_response(client_socket, &response);
    return 0;
}
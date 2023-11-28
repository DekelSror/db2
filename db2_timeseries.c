#include <string.h>
#include <sys/socket.h>

#include "utilities.h"
#include "db2_types.h"
#include "db2_mempool.h"
#include "db2.h"


static char timeseries_values[0x100000 * 20] = { 0 };

static unsigned value_offsets[20] = { 0 };

int ts_init_value_buffers(void)
{
    for (int i = 0; i < 20; i++)
    {
        value_offsets[i] = i * 0x100000;
    }
    
    return 0;
}

static uint32_t ts_value_start(int index)
{
    return 0x100000 * index;
}

typedef struct 
{
    time_t _time;
    unsigned _val_offset;
    unsigned _val_size;
} timeseries_entry_t;

typedef struct
{
    db_value_t* _key;
    unsigned _capacity;
    unsigned _size;
    timeseries_entry_t* _entries;
} timeseries_t;

static timeseries_t db[20] = { 0 };

static int next_series_index = 0;


int timeseries_create(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };
    struct db_op_ts_create_t header = op->_header._ts_create;

    if (next_series_index == 20)
    {
        response._status = 500;
        outl("already have 20 timeseries");
        send(client_socket, &response, sizeof(db_response_t), 0);
        return 1;
    }

    db_value_t* key = Mempool.allocate(header._key_size + sizeof(db_value_t));
    key->_size = header._key_size;

    send(client_socket, &response, sizeof(db_response_t), 0);
    stream_in(client_socket, key->_val, header._key_size);

    for (int i = 0; i < next_series_index; i++)
    {
        if (memcmp(key->_val, db[i]._key, header._key_size) == 0)
        {
            Mempool.free(key);
            outl("series '%s' already exists", db[i]._key->_val);
            response._body_size = i;
            send(client_socket, &response, sizeof(db_response_t), 0);

            return 0;
        }
    }
    
    db[next_series_index]._key = key;

    outl("creating timeseries '%s' index %d", key->_val, next_series_index);

    db[next_series_index]._entries = (timeseries_entry_t*)Mempool.allocate(sizeof(timeseries_entry_t) * 1024);
    db[next_series_index]._capacity = 1024;
    db[next_series_index]._size = 0;

    response._body_size = next_series_index;
    send(client_socket, &response, sizeof(db_response_t), 0);
    next_series_index++;

    return 0;
}

static int add_conditions(int ts_index, uint32_t val_size, time_t add_time)
{
    timeseries_t ts = db[ts_index];
    
    if (ts_index >= next_series_index)
    {
        outl("timeseries %d does not exist", ts_index);
        return 0;
    }

    if (ts._size == ts._capacity)
    {
        outl("ts_add size == capacity for ts '%s'. future - try to allocate more memory", ts._key->_val);
        return 0;
    }
    
    if (ts._size > 0 && ts._entries[ts._size]._time >= add_time)
    {
        outl("ts_add cannot add earlier to latest; latest(%ld) - new(%ld) = %ld", 
            ts._entries[ts._size]._time, 
            add_time, 
            ts._entries[ts._size]._time - add_time
        );

        return 0;
    }
    if (next_series_index < ts_index) {
        outl("ts_add op has ts %d but ts only has up to %d", ts_index, next_series_index);
        return 0;
    }
    if (value_offsets[ts_index] + val_size > ts_value_start(ts_index + 1))
    {
        outl("ts_add out of space for values!");
        return 0;
    }

    return 1;
}

int timeseries_add(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };
    time_t add_time = time(NULL);
    struct db_op_ts_add_t header = op->_header._ts_add;
    timeseries_t* ts = db + header._ts;

    outl("timeseries_add called at %ld", add_time);

    if (add_conditions(header._ts, header._val_size, add_time))
    {
        send(client_socket, &response, sizeof(db_response_t), 0);
        
        ts->_entries[ts->_size]._time = add_time;
        ts->_entries[ts->_size]._val_offset =  value_offsets[header._ts];
        ts->_entries[ts->_size]._val_size = header._val_size;

        stream_in(client_socket, timeseries_values + value_offsets[header._ts], header._val_size);
        value_offsets[header._ts] += header._val_size;

        ts->_size++;
        send(client_socket, &response, sizeof(db_response_t), 0);
        return 0;
    } 
    else
    {
        response._status = 500;
        send(client_socket, &response, sizeof(db_response_t), 0);
        return 1;
    }
}

int timeseries_get_range(db_op_t* op, int client_socket)
{

    db_response_t response = { ._status = 200 };
    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    if (header._ts == next_series_index)
    {
        return 1;
    }
    
    timeseries_t series = db[header._ts];

    time_t start = header._start;
    time_t end = header._end;

    outl("timeseries_get_range from %ld to %ld range %ld", start, end, end - start);
    
    unsigned index = 0;

    while (series._entries[index]._time < start)
    {
        if (index == series._size)
        {
            response._status = 404;
            send(client_socket, &response, sizeof(db_response_t), 0);
            return 1;
        }

        index++;
    }

    unsigned start_index = index;
    unsigned to_send = 0;

    while (series._entries[index]._time < end)
    {
        if (index == series._size)
        {
            break;
        }
        to_send += series._entries[index]._val_size;
        index++;
    }

    outl("get_range sending successful response");
    response._body_size = to_send;
    send(client_socket, &response, sizeof(db_response_t), 0);
    outl("get_range streaming out result");
    ssize_t sent = stream_out(client_socket, timeseries_values + series._entries[start_index]._val_offset, to_send);
    outl("get_range done (200), back to server");
    response._body_size = sent;
    send(client_socket, &response, sizeof(db_response_t), 0);
    
    return 0;
}
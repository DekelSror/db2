#include <string.h>

#include "utilities.h"
#include "db2_types.h"
#include "db2_mempool.h"
#include "db2.h"

typedef struct 
{
    time_t _time;
    db_value_t* _val;
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

static char response_buf[0x4000] = { 0 };

int timeseries_create(db_op_t* op, int client_socket)
{
    struct db_op_ts_create_t header = op->_header._ts_create;

    if (next_series_index == 20)
    {
        outl("already have 20 timeseries");
        return -1;
    }

    db_value_t* key = Mempool.allocate(header._key_size + sizeof(db_value_t));
    key->_size = header._key_size;
    stream_in(client_socket, key->_val, header._key_size);

    for (int i = 0; i < next_series_index; i++)
    {
        if (memcmp(key->_val, db[i]._key, header._key_size) == 0)
        {
            Mempool.free(key);
            outl("series '%s' already exists", db[i]._key->_val);
            return i;
        }
    }
    
    db[next_series_index]._key = key;

    outl("creating timeseries '%s' index %d", db[next_series_index]._key->_val, next_series_index);

    db[next_series_index]._entries = Mempool.allocate(sizeof(timeseries_entry_t) * 1024);
    db[next_series_index]._capacity = 1024;
    db[next_series_index]._size = 1;

    db[next_series_index]._entries[0]._time = time(NULL);
    db[next_series_index]._entries[0]._val = NULL;

    int rv = next_series_index;
    next_series_index++;

    return rv;
}

static int add_conditions(int ts_index, time_t add_time)
{
    timeseries_t ts = db[ts_index];

    if (ts_index >= next_series_index)
    {
        outl("timeseries %d does not exist", ts_index);
        return 0;
    }

    if (ts._size == ts._capacity)
    {
        outl("ts_add size == capacity for ts %d. future - try to allocate more memory", ts_index);
        return 0;
    }
    if (ts._entries[ts._size - 1]._time >= add_time)
    {
        outl("ts_add cannot add earlier to latest latest(%ld) - new(%ld) = %ld", 
            ts._entries[ts._size - 1]._time, 
            add_time, 
            ts._entries[ts._size - 1]._time - add_time
        );

        return 0;
    }
    if (next_series_index < ts_index) {
        outl("ts_add op has ts %d but ts only has up to %d", ts_index, next_series_index);
        return 0;
    }

    return 1;
}

int timeseries_add(db_op_t* op, int client_socket)
{
    time_t add_time = time(NULL);
    struct db_op_ts_add_t header = op->_header._ts_add;
    timeseries_t ts = db[header._ts];

    if (add_conditions(header._ts, add_time))
    {
        db_value_t* val = Mempool.allocate(header._val_size);
        val->_size = header._val_size;
        stream_in(client_socket, val->_val, header._val_size);

        ts._entries[ts._size]._time = add_time;
        ts._entries[ts._size]._val = val;

        ts._size++;
    }

    return 0;
}

int timeseries_get_range(db_op_t* op, int client_socket)
{
    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    if (header._ts == next_series_index)
    {
        return 1;
    }
    
    timeseries_t series = db[header._ts];

    time_t start = header._start;
    time_t end = header._end;
    
    unsigned index = 0;

    while (series._entries[index]._time < start)
    {
        if (index == series._size)
        {
            return 1;
        }

        index++;
    }
    // unsigned start_index = index;

    db_value_t* rv = (db_value_t*)response_buf;


    while (series._entries[index]._time < end)
    {
        memmove(rv->_val + rv->_size, series._entries[index]._val->_val, series._entries[index]._val->_size);
        rv->_size += series._entries[index]._val->_size;
        index++;
    }

    
    return 0;
}
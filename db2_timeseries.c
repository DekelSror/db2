#include <string.h>

#include "db2_types.h"
#include "db2_mempool.h"
#include "db2.h"

static struct timeseries_t
{
    char _key[128];
    unsigned _key_len;
    unsigned _capacity;
    unsigned _size;
    struct timeseries_entry_t
    {
        time_t _time;
        db_value_t* _value;
    }* _entries;
} db[20] = { 0 };

static int next_series_index = 0;

int timeseries_create(db_op_t* op)
{
    struct db_op_ts_create_t header = op->_header._ts_create;

    if (next_series_index == 20)
    {
        outl("already have 20 timeseries");
        return -1;
    }

    db[next_series_index]._key_len = header._key_size;
    memmove(db[next_series_index]._key, op->_body, header._key_size);

    outl("creating timeseries '%s' index %d", db[next_series_index]._key, next_series_index);

    // design mistake - mempool is for db_value_t objects, not timeseries_entry
    db[next_series_index]._entries = Mempool.allocate(sizeof(struct timeseries_entry_t) * 1024);
    db[next_series_index]._capacity = 1024;
    db[next_series_index]._size = 1;

    db[next_series_index]._entries[0]._time = time(NULL);
    db[next_series_index]._entries[0]._value = NULL;

    int rv = next_series_index;
    next_series_index++;

    return rv;
}


static int add(int i, void* val, unsigned val_len, time_t add_time)
{
    if (db[i]._size == db[i]._capacity)
    {
        outl("ts_add size == capacity for ts %d. future - try to allocate more memory", i);
        return 1;
    }

    if (db[i]._entries[db[i]._size - 1]._time >= add_time)
    {
        outl("ts_add cannot add earlier to latest latest(%ld) - new(%ld) = %ld", 
            db[i]._entries[db[i]._size - 1]._time, 
            add_time, 
            db[i]._entries[db[i]._size - 1]._time - add_time
        );

        return 1;
    }

    db_value_t* val_block = (db_value_t*)Mempool.allocate(sizeof(db_value_t) + val_len);

    val_block->_size = val_len;
    memmove(val_block->_val, val, val_len);

    db[i]._entries[db[i]._size]._time = add_time;
    db[i]._entries[db[i]._size]._value = val_block;

    db[i]._size++;

    return 0;
}

int timeseries_add(db_op_t* op)
{
    time_t add_time = time(NULL);
    struct db_op_ts_add_t header = op->_header._ts_add;
    
    if (next_series_index < header._ts) {
        outl("ts_add op has ts %d but ts only has up to %d", header._ts, next_series_index);
        return 1;
    }

    return add(header._ts, op->_body, header._val_size, add_time);
}

int timeseries_get_range(db_op_t* op)
{
    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    if (header._ts == next_series_index)
    {
        return 1;
    }
    
    struct timeseries_t series = db[header._ts];

    time_t start = (time_t)op->_body;
    time_t end = (time_t)(op->_body + 8);
    
    unsigned index = 0;

    while (series._entries[index]._time < start)
    {
        if (index == series._size)
        {
            return 1;
        }

        index++;
    }

    db_value_t* rv = { 0 };

    while (series._entries[index]._time < end)
    {
        memmove(rv->_val + rv->_size, series._entries[index]._value->_val, series._entries[index]._value->_size);
        rv->_size += series._entries[index]._value->_size;
        index++;
    }    
    
    return 0;
}

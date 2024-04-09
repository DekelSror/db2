#include <string.h>
#include <sys/socket.h>

#include "utilities.h"
#include "db2_mempool.h"
#include "db2_timeseries.h"

typedef struct
{
    db2_value_t* _key;
    unsigned _size;
    timeseries_entry_t _entries[db2_max_timeseries_entries];
} timeseries_t;

static struct ts_range_t ts_range_params(struct db_op_ts_get_range_t header);

static timeseries_t db[db2_max_timeseries] = { 0 };
static int next_series_index = 0;
struct ts_range_t
{
    uint32_t _start_index;
    uint32_t _end_index;
};


int timeseries_can_create(struct db_op_ts_create_t header)
{

    if (next_series_index == db2_max_timeseries)
    {
        return 0;
    }

    if (!Mempool.has(header._key_size))
    {
        return 0;
    }

    return 1;
}


int timeseries_create(db2_value_t* name)
{
    for (int i = 0; i < next_series_index; i++)
    {
        if (memcmp(name->_val, db[i]._key, name->_size) == 0)
        {
            Mempool.free(name);
            outl("series '%s' already exists", db[i]._key->_val);
            
            return i;
        }
    }
    
    db[next_series_index]._key = name;

    outl("creating timeseries '%s' index %d", name->_val, next_series_index);

    db[next_series_index]._size = 0;

    int rv = next_series_index;

    next_series_index++;

    return rv;

}

int timeseries_add(struct db_op_ts_add_t header)
{
    db2_time_t add_time = db2_now();
    timeseries_t* ts = db + header._ts;


    int can_add = header._ts < next_series_index; 
    can_add = can_add && db[header._ts]._size < db2_max_timeseries_entries;

    if (can_add)
    {
        ts->_entries[ts->_size]._time = add_time;
        ts->_entries[ts->_size]._val = header._val;
        ts->_size++;
    } 
    
    
    return can_add;
}

db2_time_t timeseries_start_end(struct db_op_ts_start_end_t header)
{
    return header._type == 0 ? 
        db[header._ts]._entries[0]._time : 
        db[header._ts]._entries[db[header._ts]._size - 1]._time;
}

struct ts_slice_t timeseries_get_range(struct db_op_ts_get_range_t header)
{
    db2_ts_descriptor_t ts = header._ts;

    struct ts_range_t range = ts_range_params(header);

    if (range._end_index == range._start_index)
    {
        return (struct ts_slice_t){
            .start = NULL,
            .count = 0
        };
    }

    return (struct ts_slice_t){
        .start =  db[ts]._entries + range._start_index,
        .count = range._end_index - range._start_index
    };
}

// ._start_index == ._end_index on result means failure for now
static struct ts_range_t ts_range_params(struct db_op_ts_get_range_t header)
{
    timeseries_t series = db[header._ts];
    db2_time_t start = header._start;
    db2_time_t end = header._end;

    struct ts_range_t res = { 0 };
    if (start > series._entries[series._size - 1]._time)
    {
        return res;
    }

    uint32_t index = 0;

    while (index < series._size)
    {
        if (series._entries[index]._time >= start)
        {
            res._start_index = index;
            break;
        }
        index++;
    }

    while (index < series._size)
    {
        if (series._entries[index]._time >= end)
        {
            res._end_index = index;
            break;
        }

        index++;
    }
    
    return res;
}
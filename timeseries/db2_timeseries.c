#include <string.h>
#include <sys/socket.h>

#include "utilities.h"
#include "db2_mempool.h"
#include "db2_timeseries.h"

typedef struct
{
    db2_value_t* key;
    unsigned size;
    timeseries_entry_t entries[db2_max_timeseries_entries];
} timeseries_t;

static struct ts_range_t ts_range_params(struct db_op_ts_get_range_t header);

static timeseries_t db[db2_max_timeseries] = { 0 };
static int next_series_index = 0;
struct ts_range_t
{
    uint32_t start_index;
    uint32_t end_index;
};


int timeseries_can_create(struct db_op_ts_create_t header)
{

    if (next_series_index == db2_max_timeseries)
    {
        return 0;
    }

    if (!Mempool.has(header.key_size))
    {
        return 0;
    }

    return 1;
}


int timeseries_create(db2_value_t* name)
{
    for (int i = 0; i < next_series_index; i++)
    {
        if (memcmp(name->val, db[i].key, name->size) == 0)
        {
            Mempool.free(name);
            outl("series '%s' already exists", db[i].key->val);
            
            return i;
        }
    }
    
    db[next_series_index].key = name;

    outl("creating timeseries '%s' index %d", name->val, next_series_index);

    db[next_series_index].size = 0;

    int rv = next_series_index;

    next_series_index++;

    return rv;

}

int timeseries_add(struct db_op_ts_add_t header)
{
    db2_time_t add_time = db2_now();
    timeseries_t* ts = db + header.ts;


    int can_add = header.ts < next_series_index; 
    can_add = can_add && db[header.ts].size < db2_max_timeseries_entries;

    if (can_add)
    {
        ts->entries[ts->size].time = add_time;
        ts->entries[ts->size].val = header.val;
        ts->size++;
    } 
    
    
    return can_add;
}

db2_time_t timeseries_start_end(struct db_op_ts_start_end_t header)
{
    return header.type == 0 ? 
        db[header.ts].entries[0].time : 
        db[header.ts].entries[db[header.ts].size - 1].time;
}

struct ts_slice_t timeseries_get_range(struct db_op_ts_get_range_t header)
{
    db2_ts_descriptor_t ts = header.ts;

    struct ts_range_t range = ts_range_params(header);

    if (range.end_index == range.start_index)
    {
        return (struct ts_slice_t){
            .start = NULL,
            .count = 0
        };
    }

    return (struct ts_slice_t){
        .start =  db[ts].entries + range.start_index,
        .count = range.end_index - range.start_index
    };
}

// .start_index == .end_index on result means failure for now
static struct ts_range_t ts_range_params(struct db_op_ts_get_range_t header)
{
    timeseries_t series = db[header.ts];
    db2_time_t start = header.start;
    db2_time_t end = header.end;

    struct ts_range_t res = { 0 };
    if (start > series.entries[series.size - 1].time)
    {
        return res;
    }

    uint32_t index = 0;

    while (index < series.size)
    {
        if (series.entries[index].time >= start)
        {
            res.start_index = index;
            break;
        }
        index++;
    }

    while (index < series.size)
    {
        if (series.entries[index].time >= end)
        {
            res.end_index = index;
            break;
        }

        index++;
    }
    
    return res;
}
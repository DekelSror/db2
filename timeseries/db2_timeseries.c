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


static timeseries_t db[db2_max_timeseries] = { 0 };
static int next_series_index = 0;


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

#ifndef db2_client_h
#define db2_client_h

#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "db2_types.h"

struct db2_client_t
{
    int(*connect)(void);
    void(*stop)(void);
    //
    int(*kv_insert)(char* key, uint32_t key_len, void* val, uint32_t val_len);
    int(*kv_remove)(char* key, uint32_t key_len);
    // find allocates memory for the returned value; it is up to the user to free it
    void*(*kv_find)(char* key, uint32_t key_len);
    //
    // timeseries_create will return the descriptor if the series already exists
    db2_ts_descriptor_t(*timeseries_create)(char* key, uint32_t key_len);
    int(*timeseries_add)(db2_ts_descriptor_t ts, double val);
    timeseries_entry_t*(*timeseries_get_range)(db2_ts_descriptor_t ts, db2_time_t start, db2_time_t end);
    db2_time_t(*timeseries_start)(db2_ts_descriptor_t ts);
    db2_time_t(*timeseries_end)(db2_ts_descriptor_t ts);
};

extern const struct db2_client_t Db2;


#endif // db2_client_h
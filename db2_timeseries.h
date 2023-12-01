#ifndef db2_internal_timeseries_h
#define db2_internal_timeseries_h

#include "db2_types.h"

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

#endif // db2_internal_timeseries_h
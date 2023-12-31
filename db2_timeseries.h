#ifndef db2_timeseries_h
#define db2_timeseries_h

#include "db2_types.h"
#include "db2_types.h"
#include "db2_time.h"

#define db2_max_timeseries (20)
#define db2_max_timeseries_entries (0x1000)

#ifndef timeseries_entry_t
typedef struct 
{
    db2_time_t _time;
    double _val;
} timeseries_entry_t;
#endif



#endif // db2_timeseries_h
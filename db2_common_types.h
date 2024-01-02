#ifndef db2_common_types
#define db2_common_types

// commomn types - types the end user needs to know about

#ifndef uint64_t
#include <stdint.h>
#endif

#ifndef db2_time_t
typedef uint64_t db2_time_t;
#endif

#ifndef db2_ts_descriptor_t
typedef int db2_ts_descriptor_t;
#endif


#ifndef timeseries_entry_t
typedef struct 
{
    db2_time_t _time;
    double _val;
} timeseries_entry_t;
#endif

#endif // db2_common_types
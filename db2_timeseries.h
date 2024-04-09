#ifndef db2_timeseries
#define db2_timeseries

#include "db2_types.h"

#define db2_max_timeseries (20)
#define db2_max_timeseries_entries (0x1000)

typedef struct
{
    db2_value_t* _key;
    unsigned _size;
    timeseries_entry_t _entries[db2_max_timeseries_entries];
} timeseries_t;

struct ts_slice_t {timeseries_entry_t* start; uint32_t count;};

int timeseries_can_create(struct db_op_ts_create_t header);
int timeseries_create(db2_value_t* name);
int timeseries_add(struct db_op_ts_add_t header);
db2_time_t timeseries_start_end(struct db_op_ts_start_end_t header);
struct ts_slice_t timeseries_get_range(struct db_op_ts_get_range_t header);



#endif // db2_timeseries
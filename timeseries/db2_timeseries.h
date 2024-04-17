#ifndef db2_timeseries
#define db2_timeseries

#include "db2_types.h"

#define db2_max_timeseries (20)
#define db2_max_timeseries_entries (0x1000)

int timeseries_can_create(struct db_op_ts_create_t header);
db2_ts_descriptor_t timeseries_create(db2_value_t* name);
int timeseries_add(struct db_op_ts_add_t header);

#endif // db2_timeseries
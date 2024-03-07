

#include "db2_types.h"

const struct db_op_types Db2OpTypes = 
{
    ._insert = 0,
    ._find = 1,
    ._remove = 2,
    ._ts_create = 3,
    ._ts_add = 4,
    ._ts_get_range = 5,
    ._ts_start_end = 6,
    ._num_ops = 7
};
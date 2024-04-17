

#include "db2_types.h"

const struct db_op_types Db2OpTypes = 
{
    .insert = 0,
    .find = 1,
    .remove = 2,
    .ts_create = 3,
    .ts_add = 4,
    .num_ops = 5
};
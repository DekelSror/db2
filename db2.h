#ifndef db2_h
#define db2_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "db2_types.h"


#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

#define db2_num_entries 0x100

db_value_t* handle_find(db_op_t* op);
int handle_remove(db_op_t* op);
int handle_insert(db_op_t* op);


int timeseries_create(db_op_t* op);
int timeseries_add(db_op_t* op);
int timeseries_get_range(db_op_t* op);



#endif // db2_h
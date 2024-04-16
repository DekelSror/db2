#ifndef db2_ts_handlers
#define db2_ts_handlers

#include "db2_types.h"

int handle_create(db_op_t* op, int client_socket);
int handle_timeseries_add(db_op_t* op, int client_socket);
int handle_ts_start_end(db_op_t* op, int client_socket);
int handle_timeseries_get_range(db_op_t* op, int client_socket);



#endif // db2_ts_handlers
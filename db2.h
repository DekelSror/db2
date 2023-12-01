#ifndef db2_main_header_h
#define db2_main_header_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "db2_types.h"


#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

#define db2_num_entries 0x100

int handle_find(db_op_t* op, int client_socket);
int handle_remove(db_op_t* op, int client_socket);
int handle_insert(db_op_t* op, int client_socket);


typedef int db2_ts_descriptor_t;

int ts_init_value_buffers(void);
int timeseries_create(db_op_t* op, int client_socket);
int timeseries_add(db_op_t* op, int client_socket);
int timeseries_get_range(db_op_t* op, int client_socket);



#endif // db2_main_header_h
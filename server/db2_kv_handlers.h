#ifndef db2_kv_handlers
#define db2_kv_handlers

#include "db2_types.h"


int handle_insert(db_op_t *op, int client_socket);
int handle_remove(db_op_t *op, int client_socket);
int handle_find(db_op_t *op, int client_socket);


#endif // db2_kv_handlers



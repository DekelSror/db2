#ifndef db2_kv
#define db2_kv

#include "db2_types.h"

#define db2_num_entries 0x10

db2_value_t* kv_find(struct db_op_find_t header);
int kv_remove(struct db_op_remove_t header);


int kv_insert(uint64_t key_hash, db2_value_t* key, db2_value_t* val);
int kv_can_insert(struct db_op_insert_t header);

#endif // db2_kv
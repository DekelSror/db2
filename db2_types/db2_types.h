#ifndef db2_types
#define db2_types

#include <stdint.h>
#include "db2_time.h"

// db2_types - common types for server and client

typedef int db2_ts_descriptor_t;
typedef uint64_t db2_time_t;

typedef struct 
{
    db2_time_t time;
    double val;
} timeseries_entry_t;


struct db_op_types 
{
    int insert;
    int find;
    int remove;
    int ts_create;
    int ts_add;
    int num_ops;
};

extern const struct db_op_types Db2OpTypes;

typedef struct
{
    uint32_t size;
    char val[];
} db2_value_t;

typedef struct 
{
    int status;
    int body_size;
} db_response_t;

struct db_ts_create_response
{
    int status;
    db2_ts_descriptor_t ts;
};

typedef struct
{
    uint64_t hash;
    db2_value_t* key;
    db2_value_t* val;
} db_entry_t;

struct db_op_insert_t
{
    uint32_t key_size;
    uint32_t val_size;
    uint64_t key_hash;
};

struct db_op_remove_t
{
    uint64_t key_hash;
};

struct db_op_find_t
{
    uint64_t key_hash;
};

struct db_op_ts_create_t
{
    uint32_t key_size;
};

struct db_op_ts_add_t 
{
    int ts;
    double val;
};

union db_op_header_t 
{
    struct db_op_insert_t insert;
    struct db_op_find_t find;
    struct db_op_remove_t remove;
    struct db_op_ts_create_t ts_create;
    struct db_op_ts_add_t ts_add;
};

typedef struct
{
    int op;
    union db_op_header_t header;
} db_op_t;


#endif // db2_types
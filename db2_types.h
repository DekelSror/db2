#ifndef db2_types
#define db2_types

#include <stdint.h>
#include "db2_time.h"

// db2_types - common types for server and client

typedef int db2_ts_descriptor_t;


struct db_op_types 
{
    int _insert;
    int _find;
    int _remove;
    int _ts_create;
    int _ts_add;
    int _ts_get_range;
    int _ts_start_end;
    int _num_ops;
};

extern const struct db_op_types Db2OpTypes;

typedef struct
{
    uint32_t _size;
    char _val[];
} db_value_t;

typedef struct 
{
    int _status;
    int _body_size;
} db_response_t;

struct db_ts_create_response
{
    int _status;
    db2_ts_descriptor_t _ts;
};

typedef struct
{
    uint64_t _hash;
    db_value_t* _key;
    db_value_t* _val;
} db_entry_t;

struct db_op_insert_t
{
    uint32_t _key_size;
    uint32_t _val_size;
    uint64_t _key_hash;
};

struct db_op_remove_t
{
    uint64_t _key_hash;
};

struct db_op_find_t
{
    uint64_t _key_hash;
};

struct db_op_ts_create_t
{
    uint32_t _key_size;
};

struct db_op_ts_add_t 
{
    int _ts;
    double _val;
};

struct db_op_ts_get_range_t
{
    int _ts;
    db2_time_t _start;
    db2_time_t _end;
};

struct db_op_ts_start_end_t
{
    int _ts;
    int _type; // 0 start, 1 end
};

union db_op_header_t 
{
    struct db_op_insert_t _insert;
    struct db_op_find_t _find;
    struct db_op_remove_t _remove;
    struct db_op_ts_create_t _ts_create;
    struct db_op_ts_add_t _ts_add;
    struct db_op_ts_get_range_t _ts_get_range;
    struct db_op_ts_start_end_t _ts_start_end;
};

typedef struct
{
    int _op;
    union db_op_header_t _header;
} db_op_t;


#endif // db2_types
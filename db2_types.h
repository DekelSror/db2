#ifndef db2_types
#define db2_types

#include <time.h>
#include <stdint.h>

enum db_op_type_e { op_insert, op_find, op_remove, op_ts_create, op_ts_add, op_ts_get_range };

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
    char pad[16];
};

struct db_op_remove_t
{
    uint64_t _key_hash;
    char pad[16];
};

struct db_op_find_t
{
    uint64_t _key_hash;
    char pad[16];
};

struct db_op_ts_create_t
{
    uint32_t _key_size;
    char pad[20];
};

struct db_op_ts_add_t 
{
    int _ts;
    uint32_t _val_size;
    char pad[16];
};

struct db_op_ts_get_range_t
{
    int _ts;
    char pad[4];
    time_t _start;
    time_t _end;
};

// insert, remove and find need exactly 8 bytes of body
// but this will be difficult to keep up
union db_op_header_t 
{
    struct db_op_insert_t _insert;
    struct db_op_find_t _find;
    struct db_op_remove_t _remove;
    struct db_op_ts_create_t _ts_create;
    struct db_op_ts_add_t _ts_add;
    struct db_op_ts_get_range_t _ts_get_range;
};

typedef struct
{
    enum db_op_type_e _op;
    union db_op_header_t _header;
} db_op_t;


#endif // db2_types
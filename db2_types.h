#ifndef db_types_h
#define db_types_h

#include <stdint.h>

typedef enum { op_insert, op_find, op_remove } db_op_type_e;

typedef enum { idle, transaction_started, transaction_rejected } db_transaction_state_e;

typedef struct
{
    uint32_t _size;
    char _val[];
} db_value_t;

typedef struct
{
    int _status;
    char _body[];
} db_response_t;

typedef struct {
    uint64_t _hash;
    db_value_t* _key;
    db_value_t* _val;
} db_entry;

typedef struct
{
    uint32_t _key_size;
    uint32_t _val_size;
} db_op_insert_t;

typedef struct
{
    uint32_t _key_size;
    uint32_t _pad;
} db_op_remove_t;

typedef db_op_remove_t db_op_find_t;

// insert, remove and find need exactly 8 bytes of body
// but this will be difficult to keep
typedef union 
{
    db_op_insert_t _insert;
    db_op_find_t _find;
    db_op_remove_t _remove;
} db_op_header_t;

typedef struct
{
    db_op_type_e _op;
    uint32_t _size;
    db_op_header_t _header;
    char _body[];
} db_op_t;

#endif // db_types_h
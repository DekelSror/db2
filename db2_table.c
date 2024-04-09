#include <string.h>
#include <stdarg.h>

#include "db2_types.h"
#include "db2_mempool.h"
#include "db2_table.h"

typedef struct 
{
    db2_value_t* _name;
    enum column_types_e _type;
    void* _validator;
    void* _validator_arg;
} table_col_t;

static table_col_t factory_cols[0x100];

int num_cols = 0;
int factory_is_ready = 1;


typedef struct
{
    table_col_t* _cols;
    int _num_cols;
} db2_table_t;

static table_factory_t create(char* name, size_t name_len, int persist)
{
    if (!factory_is_ready)
    {
        return NULL;
    }

    factory_is_ready = 0;
    return 1;
}

static int add_column (table_factory_t table, 
    char* column_name, size_t name_len, 
    enum column_types_e column_type, 
    int(*validator)(void*), 
    void* validator_arg)
{
    // verify we have the memory
    factory_cols[num_cols] = Mempool.allocate(sizeof(db2_value_t) + name_len);
    factory_cols[num_cols]._type = column_type;
    memmove(factory_cols[num_cols]._name->_val, column_name, name_len);
    factory_cols[num_cols]._validator = validator;
    factory_cols[num_cols]._validator_arg = validator_arg;

    num_cols += 1;

    return 0;
}


static table_descriptor_t finalize (table_factory_t table)
{
    if (factory_is_ready)
    {

    }

    if (num_cols == 0)
    {

    }

    factory_is_ready = 1;

}

#define translate_type(coltype) int

static int insert(table_descriptor_t table_d, ...)
{
    db2_table_t* table = table_d;

    va_list args;
    va_start(args, table->_num_cols);

    
    for (size_t i = 0; i < table->_num_cols; i++) 
    {
        enum column_types_e next_type = table->_cols[i]._type;

        va_arg(args, translate_type(table->_cols[i]._type));
    }
    
}

#ifndef db2_table_h
#define db2_table_h

#include <stddef.h>

typedef void* table_factory_t;
typedef void* table_descriptor_t;

enum column_types_e {number, string, time, json};

struct db2_table_api_t 
{
    // schema related
    table_factory_t(*create)(char* name, size_t name_len, int persist);
    int(*add_column)(table_factory_t table, char* column_name, enum column_types_e column_type, int(*validator)(void*), void* validator_arg);
    table_descriptor_t(*finalize)(table_factory_t table);
    //
    int(*insert)(table_descriptor_t table, ...); // va_args are the cell values 
    int(*query)(table_descriptor_t table, char* query, char* results_buf);
};

extern const struct db2_table_api_2 Db2Table;

#endif // db2_table_h
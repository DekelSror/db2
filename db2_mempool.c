#include <stdlib.h>


#include "db2_mempool.h"

static size_t current_size = 0;
static size_t limit = 0x400 * 0x400 * 0x400;


static int has(size_t size)
{
    return limit - current_size >= size;
}

static db_value_t* db2_allocate(size_t size)
{
    db_value_t* res = NULL;

    if (has(size))
    {
        res = (db_value_t*)malloc(size);

        if (res != NULL)
        {
            current_size += size;
        }
    }

    return res;
}

static void db2_free(db_value_t* block)
{
    if (block != NULL)
    {
        current_size -= block->_size;
        free(block);
    }
}

static size_t db2_heap_size(void)
{
    return current_size;
}


const mempool_t Mempool = {
    .has = has,
    .allocate = db2_allocate,
    .free = db2_free,
    .heap_size = db2_heap_size
};



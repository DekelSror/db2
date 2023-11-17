#ifndef db2_mempool
#define db2_mempool

#include <stddef.h>
#include "db2_types.h"


//

typedef struct
{
    db_value_t*(*allocate)(size_t size);
    void(*free)(db_value_t*);
    size_t(*heap_size)(void);
} mempool_t;

extern const mempool_t Mempool;

#endif // db2_mempool
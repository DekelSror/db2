#ifndef db2_client_h
#define db2_client_h

#include <stdint.h>
#include <stddef.h>


typedef int db2_ts_descriptor_t;

typedef struct 
{
    int(*connect)(void);
    void(*stop)(void);
    int(*message)(const char*, size_t);
    //
    int(*insert)(char* key, uint32_t key_len, void* val, uint32_t val_len);
    int(*remove)(char* key, uint32_t key_len);
    void*(*find)(char* key, uint32_t key_len);
    //
    db2_ts_descriptor_t(*timeseries_create)(char* key, uint32_t key_len);
    int(*timeseries_add)(db2_ts_descriptor_t ts, void* val, uint32_t val_len);
    int(*timeseries_get_range)(db2_ts_descriptor_t ts, time_t start, time_t end, void* buf);
} db2_client_t;

extern const db2_client_t Db2;


#endif // db2_client_h
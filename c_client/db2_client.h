#ifndef db2_client_h
#define db2_client_h

#include <stdint.h>
#include <stddef.h>
#include <time.h>


typedef int db2_ts_descriptor_t;
typedef void* timeseries_entry_t;
typedef void* db2_time_t;

struct db2_client_t
{
    int(*connect)(void);
    void(*stop)(void);
    int(*message)(const char*, size_t);
    //
    int(*insert)(char* key, uint32_t key_len, void* val, uint32_t val_len);
    int(*remove)(char* key, uint32_t key_len);
    // find allocates memory for the returned value; it is up to the user to free it
    void*(*find)(char* key, uint32_t key_len);
    //
    // timeseries_create will return the descriptor if the series already exists
    db2_ts_descriptor_t(*timeseries_create)(char* key, uint32_t key_len);
    int(*timeseries_add)(db2_ts_descriptor_t ts, double val);
    timeseries_entry_t*(*timeseries_get_range)(db2_ts_descriptor_t ts, time_t start, time_t end);
    db2_time_t(*timeseries_start)(b2_ts_descriptor_t ts);
    db2_time_t(*timeseries_end)(b2_ts_descriptor_t ts);
} db2_client_t;

extern const struct db2_client_t Db2;


#endif // db2_client_h
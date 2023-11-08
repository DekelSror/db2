#ifndef db2_client_h
#define db2_client_h

#include <stdint.h>



typedef struct 
{
    int(*connect)(void);
    void(*stop)(void);
    int(*message)(const char*, size_t);
    int(*insert)(char* key, uint32_t key_len, char* val, uint32_t val_len);
    int(*remove)(char* key, uint32_t key_len);
    void*(*find)(char* key, uint32_t key_len);
} db2_client_t;

extern const db2_client_t Db2;


#endif // db2_client_h
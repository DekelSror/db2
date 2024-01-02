#ifndef db2_driver2
#define db2_driver2

#include <stdint.h>

typedef struct
{
    int _a;
    int _b;
    int _c;
    int _d;
} user_data_t;



void generate_ud(user_data_t* buf);
uint32_t generate_key(char* buf);

int insert_find_test(uint32_t test_size);
int large_value_test(void);
int full_table_test(void);
int timeseries_test(char* name, size_t name_len, unsigned test_size);


#endif // db2_driver2
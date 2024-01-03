#include <string.h>
#include <stdlib.h>

#include "db2_client.h"
#include "driver2.h"

static void repr_udata(user_data_t* ud)
{
    printf("userdata at %p : a=%d b=%d c=%d d=%d\n", ud, ud->_a, ud->_b, ud->_c, ud->_d);
}

int insert_find_test(uint32_t test_size)
{
// setup
    if (Db2.connect() != 200)
    {
        outl("connect trouble");
        return -1;
    }

    user_data_t* values = (user_data_t*)malloc(test_size * sizeof(user_data_t));
    char** keys = malloc(test_size * 16);
    uint32_t* key_lengths = malloc(test_size * sizeof(uint32_t));
    
    if ((values == NULL) || (keys == NULL) || (key_lengths == NULL))
    {
        outl("malloc trouble");
        return -2;
    }
    
    int errors = 0;
    char gkey[16] = {0};

// generate
    for (uint32_t i = 0; i < test_size; i++)
    {
        key_lengths[i] = generate_key(gkey);
        keys[i] = malloc(key_lengths[i]);
        memmove(keys[i], gkey, key_lengths[i]);
        generate_ud(values + i);

        outl("generated key '%s'", keys[i]);
        repr_udata(values + i);
    }

    outl("data generated\n\n\n");
// insert
    for (uint32_t i = 0; i < test_size; i++)
    {
        int res = Db2.insert(keys[i], key_lengths[i], (char*)(values + i), sizeof(user_data_t));

        if (res != 200)
        {
            outl("insert error with key '%s'", keys[i]);
            errors++;
        }
    }

    outl("insert done\n\n\n");
// find
    for (uint32_t i = 0; i < test_size; i++)
    {
        user_data_t* res = (user_data_t*)Db2.find(keys[i], key_lengths[i]);

        if (res == NULL)
        {
            outl("find error with key '%s'", keys[i]);
            errors++;
        }

        const int cmp_res = memcmp(values + i, res, sizeof(user_data_t));
        if (cmp_res != 0)
        {
            errors++;
            outl("found worng value for key '%s'", keys[i]);
            printf("inserted - ");
            repr_udata(values + i);
            printf("found - ");
            repr_udata(res);
        }

        free(res);
    }

    outl("find done");
    

    for (uint32_t i = 0; i < test_size; i++)
    {
        free(keys[i]);
    }

    free(values);
    free(keys);
    free(key_lengths);
    
    Db2.stop();

    return errors;
}


void generate_ud(user_data_t* buf)
{
    buf->_a = rand() % 100;
    buf->_b = rand() % 100;
    buf->_c = rand() % 100;
    buf->_d = rand() % 100;
}

uint32_t generate_key(char* buf)
{
    uint32_t length = rand() % 5 + 4;

    for (size_t i = 0; i < length; i++)
    {
        buf[i] = rand() % 26 + 'A';
    }

    buf[length] = 0;

    return length + 1;
}


int full_table_test(void)
{
    Db2.connect();

    user_data_t entries[0x200] = { 0 };
    char* keys[0x200] = { 0 };
    uint32_t key_lengths[0x200] = { 0 };
    char tmp_key[0x10] = { 0 };

    for (size_t i = 0; i < 0x200; i++)
    {
        key_lengths[i] = generate_key(tmp_key);
        keys[i] = malloc(key_lengths[i]);
        memmove(keys[i], tmp_key, key_lengths[i]);

        generate_ud(entries + i);
    }

    for (size_t i = 0; i < 0x200; i++)
    {
        int res = Db2.insert(keys[i], key_lengths[i], entries + i, sizeof(user_data_t));

        if (res)
        {

        }
    }

    for (size_t i = 0; i < 0x200; i++)
    {
        free(keys[i]);
    }


    Db2.stop();
    return 0;
}
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h> // sleep
#include <stdlib.h> // rand
#include <stdio.h>
#include <time.h>

#include "db2_client.h"

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

typedef struct
{
    int _a;
    int _b;
    int _c;
    int _d;
} user_data_t;

static void repr_data(user_data_t* ud)
{
    printf("userdata at %p : a=%d b=%d c=%d d=%d\n", ud, ud->_a, ud->_b, ud->_c, ud->_d);
}

static void generate_ud(user_data_t* buf)
{
    buf->_a = rand() % 100;
    buf->_b = rand() % 100;
    buf->_c = rand() % 100;
    buf->_d = rand() % 100;
}

static uint32_t generate_key(char* buf)
{
    uint32_t length = rand() % 5 + 5;

    for (size_t i = 0; i < length; i++)
    {
        buf[i] = rand() % 26 + 'A';
    }

    buf[length] = 0;

    return length + 1;
}

static int message_test(char const* msg, size_t msg_len)
{
    Db2.connect();
    outl("client connected to db '%s'", msg);

    for (size_t i = 0; i < 10; i++)
    {   
        Db2.message(msg, msg_len);
        sleep(rand() % 3 + 1);
    }

    outl("client '%s' done", msg);

    Db2.stop();
    return 0;

}

static int insert_find_test(uint32_t test_size)
{
// setup
    if (Db2.connect() != 200)
    {
        outl("connect trouble");
        return -1;
    }

    user_data_t* values = (user_data_t*)malloc(test_size * sizeof(user_data_t));
    char** keys = malloc(test_size * sizeof(char*));
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
        repr_data(values + i);
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
            repr_data(values + i);
            printf("found - ");
            repr_data(res);
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

static int timeseries_test(char* name, size_t name_len, unsigned test_size)
{
    Db2.connect();

    int ts = Db2.timeseries_create(name, (uint32_t)name_len);

    user_data_t* entries = malloc(sizeof(user_data_t) * test_size);

    for (unsigned i = 0; i < test_size; i++)
    {
        generate_ud(entries + i);
    }

    outl("ts test connected, data generated");

    time_t range_start = time(NULL);

    for (unsigned i = 0; i < test_size; i++)
    {
        sleep(1);
        int add_res = Db2.timeseries_add(ts, entries + i, sizeof(user_data_t));

        if (add_res != 200)
        {
            outl("add problems %d", add_res);
        }
    }

    time_t range_end = time(NULL);

    user_data_t* range = (user_data_t*)Db2.timeseries_get_range(ts, range_start, range_end);

    for (unsigned i = 0; i < test_size; i++)
    {
        repr_data(entries + i);
        repr_data(range + i);
        printf("\n\n");
    }
    
    free(range); // ;]
    free(entries);
    Db2.stop();

    return 0;
}

int large_value_test(void)
{
    const char* path = "/home/dekel/Music/Dive RMIX.wav";
    struct stat info;

    stat(path, &info);

    int fd = open(path, O_RDONLY, 0444);
    void* file = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);    
    close(fd);

    Db2.connect();

    Db2.insert("large value", 12, file, info.st_size);


    void* from_db = Db2.find("large value", 12);


    if (memcmp(file, from_db, info.st_size) != 0)
    {
        outl("found something different than inserted");
    }
    else
    {
        int outfile = open("outfile.wav", O_CREAT | O_WRONLY , 0222);
        write(outfile, from_db, info.st_size);
        close(outfile);
    }

    munmap(file, info.st_size);
    free(from_db);

    Db2.stop();

    return 0;
}

int main(int argc, char const *argv[])
{
    srand(getpid() * time(NULL));

    if (argc < 2)
    {
        return 1;
    }

    char const* msg = argv[1];
    size_t msg_len = strlen(argv[1]) + 1;

    outl("******* client %s (%lu) start", msg, msg_len);

    timeseries_test(msg, msg_len, 3);

    large_value_test();

    insert_find_test(3);

    outl("******* client %s end", msg);

    return 0;
}

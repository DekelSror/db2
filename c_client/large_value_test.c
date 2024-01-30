#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "utilities.h"
#include "db2_client.h"
#include "driver2.h"


ssize_t generate_file(const char* path, size_t size)
{
    int fd = open(path, O_CREAT | O_WRONLY, 0400);

    size_t val =  (((size_t)rand()) << 32) | rand();
    while (size > 8)
    {
        write(fd, &val, sizeof(size_t));
        size -= 8;
    }

    while (size > 0)
    {
        write(fd, ((char*)(&val) + size), 1);
        size--;
    }

    close(fd);

    return 0;
}


int large_value_test(void)
{
    const char* path = "large_val_test_file";

    generate_file(path, 0x400 * 0x400);
    struct stat info;

    stat(path, &info);

    int fd = open(path, O_RDONLY, 0444);
    void* file = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);    
    close(fd);

    Db2.connect();

    Db2.kv_insert("large value", 12, file, info.st_size);


    void* from_db = Db2.kv_find("large value", 12);


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
    unlink(path);

    Db2.stop();

    return 0;
}
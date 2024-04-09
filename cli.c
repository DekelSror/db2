#include <string.h> // strlen, memcmp
#include <stdio.h> // printf
#include <stdlib.h> // free

#include "db2_client.h"


char const* command_names[] = {"set", "get", "del", }; 

int get_command_code(char const* command)
{
    for (int i = 0; i < 3; i++)
    {
        if (memcmp(command, command_names[i], 4) == 0)
        {
            return i;
        }
    }

    return -1;
}

int cli_set(char const* key, char const* value)
{
    return Db2.kv_insert((char*)key, strlen(key), (char*)value, strlen(value));
}

void* cli_get(char const* key)
{
    return Db2.kv_find((char*)key, strlen(key));
}

int cli_del(char const* key)
{
    return Db2.kv_remove((char*)key, strlen(key));
}

int one_off_commad(int argc, char const* argv[])
{
    if (argc < 2)
    {
        printf("no command specified\n");
        return 1;
    }

    char const* command = argv[1];  
    int command_code = get_command_code(command);

    if (command_code == -1)
    {
        printf("unknown command '%s'\n", command);
        return 2;
    }

    switch (command_code)
    {
    case 0:
        if (argc >= 3)
        {
            int res = cli_set(argv[2], argv[3]);

            printf("%d\n", res);
        }
        break;
    case 1:
        if (argc >= 2)
        {
            char* res = (char*)cli_get(argv[2]);
            printf("%s\n", res);
            free(res);
        }
        break;
    case 2:
        if (argc >= 2)
        {
            int res = cli_del(argv[2]);
            printf("%d\n", res);
        }
        break;
    
    default:
        break;
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    Db2.connect();
    int rv = one_off_commad(argc, argv);
    Db2.stop();

    return rv;
}
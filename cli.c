
#include <string.h>
#include <stdio.h>

#include "db2_client.h"


char const* command_names[] = {"set", "get", "del"}; 

int get_command(char const* command)
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


int cli_set(char const* args[])
{
    char const* key = args[1];
    char const* value = args[2];

    return Db2.kv_insert((char*)key, value - key, (char*)value, strlen(value));
}


void* cli_get(char const* args[])
{
    char const* key = args[1];

    return Db2.kv_find((char*)key, strlen(key));
}

int cli_del(char const* args[])
{
    char const* key = args[1];

    return Db2.kv_remove((char*)key, strlen(key));
}


// argv - cli.out set key1 value1
int main(int argc, char const *argv[])
{

    if (argc < 2)
    {
        printf("no command specified\n");
        return 1;
    }

    char const* command = argv[1];
    
    int command_code = get_command(command);

    if (command_code == -1)
    {
        printf("unknown command '%s'\n", command);
        return 2;
    }

    Db2.connect();

    switch (command_code)
    {
    case 0:
        if (argc >= 3)
        {
            int res = cli_set(argv);

            printf("%d\n", res);
        }
        break;
    case 1:
        if (argc >= 2)
        {
            char* res = (char*)cli_get(argv);
            printf("%s\n", res);
        }
        break;
    case 2:
        if (argc >= 2)
        {
            int res = cli_del(argv);
            printf("%d\n", res);
        }
        break;
    
    default:
        break;
    }
    
    Db2.stop();

    
    return 0;
}

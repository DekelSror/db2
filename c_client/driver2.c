#include <stdio.h>
#include <string.h>

#include "db2_common_types.h"
#include "db2_client.h"
#include "driver2.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    char const* msg = argv[1];
    size_t msg_len = strlen(argv[1]) + 1;

    outl("******* client %s start", msg);

    // timeseries_test(msg, msg_len, 3);

    // large_value_test();

    // insert_find_test(3);

    full_table_test();

    outl("******* client %s end", msg);

    return 0;
}

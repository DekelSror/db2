#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "db2_client.h"
#include "utilities.h"
#include "driver2.h"

double drand(void)
{
    return (double)(((long)rand() << 32) | rand());
}

int timeseries_test(char* name, size_t name_len, unsigned test_size)
{
    Db2.connect();
    srand(7);

    int ts = Db2.timeseries_create(name, (uint32_t)name_len);
    double* values = malloc(test_size * sizeof(double));

    for (unsigned i = 0; i < test_size; i++)
    {
        values[i] = drand();
    }
    

    outl("ts test connected");


    for (unsigned i = 0; i < test_size; i++)
    {
        sleep(1);
        // usleep(30 * 1000); // hopefully every 30ms
        
        int add_res = Db2.timeseries_add(ts, values[i]);

        if (add_res != 200)
        {
            outl("add problems %d", add_res);
        }
    }

    db2_time_t range_start = Db2.timeseries_start(ts);
    db2_time_t range_end = Db2.timeseries_end(ts);

    outl("ts test start time %lu end %lu", range_start, range_end);

    timeseries_entry_t* range = Db2.timeseries_get_range(ts, range_start, range_end);

    for (unsigned i = 0; i < test_size; i++)
    {
        if (values[i] != range[i]._val)
        {
            outl("value mismatch! inserted %lf got %lf", values[i], range[i]._val);
        }
    }
    
    free(range); // ;]
    free(values);
    Db2.stop();

    return 0;
}

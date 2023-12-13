#include <time.h>
#include <stdio.h>

#include "db2_time.h"


int main(void)
{
    char timebuf[30] = { 0 };
    db2_time_t t = db2_now();

    int timelen = to_iso(t, timebuf);

    printf("iso timestamp '%s', length %d\n", timebuf, timelen);


    time_t sectime = time(NULL);

    timelen = to_iso(from_time_t(&sectime), timebuf);

    printf("iso timestamp from time_t '%s', length %d\n", timebuf, timelen);


    return 0;
}

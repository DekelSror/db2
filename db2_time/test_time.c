#include <time.h>
#include <stdio.h>
#include <unistd.h>

#include "db2_time.h"


void basic_test(void)
{
    char timebuf[30] = { 0 };
    char timebuf2[30] = { 0 };
    char timebuf3[30] = { 0 };
    db2_time_t t = db2_now();
    sleep(2);
    db2_time_t t2 = db2_now();

    to_iso(t, timebuf);
    to_iso(t2, timebuf2);

    printf("iso timestamp '%s'\n", timebuf);
    time_print_all(t);

    struct tm wrety = {
        .tm_year = 2000 - 1900,
        .tm_mon = 0,
        .tm_mday = 0,
    };

    time_t wrety2 = mktime(&wrety);
    db2_time_t wrety3 = from_time_t(wrety2);

    printf("now (%lu) lt (%lu) 2000-01-01 ? %d\n", t, wrety3, t > wrety3);


    db2_time_t diff = t2 - t;
    to_iso(diff, timebuf3);

    printf("t (%s) - t2 (%s) is (%s)  \n", timebuf, timebuf2, timebuf3);

    return 0;
}


int main(void)
{
    uint64_t t = (uint64_t)-1l;


    char buf[100] = { 0 };

    to_iso(t, buf);

    printf("%s\n", buf);

    time_print_all(t);
}


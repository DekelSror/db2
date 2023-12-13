
#include <stdio.h>

#include "db2_time.h"


#pragma pack(1)
// the whole motivation is to represent time in microseconds using 64 bits
// and enable native arithmetics
typedef struct
{
    unsigned _year: 14;
    unsigned _month: 4;
    unsigned _day: 5;
    unsigned _hours: 5;
    unsigned _minutes: 6;
    unsigned _seconds: 6;
    unsigned _ms: 10;
    unsigned _us: 10;
} db2_time_internal_t;
#pragma pack()


db2_time_t from_time_t(time_t* mr_t)
{
    struct tm* parts = localtime(mr_t);

    db2_time_internal_t res = { 
        ._year = parts->tm_year + 1900,
        ._month = parts->tm_mon,
        ._day = parts->tm_mday,
        ._hours = parts->tm_hour,
        ._minutes = parts->tm_min,
        ._seconds = parts->tm_sec,
        ._ms = 0,
        ._us = 0
    };

    return *((uint64_t*)&res);
}

db2_time_t from_timespec(struct timespec* spec)
{
    struct tm* parts = localtime(&(spec->tv_sec));

    db2_time_internal_t res = { 
        ._year = parts->tm_year + 1900,
        ._month = parts->tm_mon,
        ._day = parts->tm_mday,
        ._hours = parts->tm_hour,
        ._minutes = parts->tm_min,
        ._seconds = parts->tm_sec,
        ._ms = spec->tv_nsec / 1000000,
        ._us = spec->tv_nsec / 1000
    };

    return *((uint64_t*)&res);
}

db2_time_t db2_now(void)
{
    struct timespec grandpa;

    clock_gettime(CLOCK_REALTIME, &grandpa);
    
    return from_timespec(&grandpa);
}


// "2022-09-27 18:00:00.000"
int to_iso(db2_time_t _time, char* buf)
{
    db2_time_internal_t t = *((db2_time_internal_t*)&_time);
    int i = 0;


    snprintf(buf + i, 5, "%d", t._year);
    i += 4;

    buf[i] = '-';
    i += 1;

    snprintf(buf + i, 3, "%d", t._month);
    i += 2;

    buf[i] = '-';
    i += 1;

    snprintf(buf + i, 3, "%d", t._day);
    i += 2;

    buf[i] = ' ';
    i += 1;

    snprintf(buf + i, 3, "%d", t._hours);
    i += 2;

    buf[i] = ':';
    i += 1;

    snprintf(buf + i, 3, "%d", t._minutes);
    i += 2;

    buf[i] = ':';
    i += 1;

    snprintf(buf + i, 3, "%d", t._seconds);
    i += 2;

    buf[i] = '.';
    i += 1;

    snprintf(buf + i, 4, "%d", t._ms);
    i += 3;

    buf[i] = 0;

    return i + 1;
}


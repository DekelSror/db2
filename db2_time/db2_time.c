#include <stdio.h>

#include "db2_time.h"


#pragma pack(1)
// the motivation is to represent time in microseconds using 64 bits
// and enable native arithmetic operationsm resulting in a time span
typedef struct
{
    unsigned _us: 10;
    unsigned _ms: 10;
    unsigned _seconds: 6;
    unsigned _minutes: 6;
    unsigned _hours: 5;
    unsigned _day: 5;
    unsigned _month: 4;
    unsigned _year: 14; // 10 bits for 1024 years from 1970 could be something
} db2_time_internal_t;
#pragma pack()


db2_time_t from_time_t(time_t mr_t)
{
    struct tm* parts = localtime(&mr_t);

    db2_time_internal_t res = { 
        ._year = parts->tm_year + 1900,
        ._month = parts->tm_mon + 1,
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
    time_t secs = spec->tv_sec;
    struct tm parts;
    
    gmtime_r(&secs, &parts);

    db2_time_internal_t res = { 
        ._year = parts.tm_year + 1900,
        ._month = parts.tm_mon + 1,
        ._day = parts.tm_mday,
        ._hours = parts.tm_hour,
        ._minutes = parts.tm_min,
        ._seconds = parts.tm_sec,
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


void time_print_all(db2_time_t dt)
{
    db2_time_internal_t t = *((db2_time_internal_t*)&dt);
    printf("%d-%d-%d %d:%d:%d.%d.%d\n", t._year, t._month, t._day, t._hours, t._minutes, t._seconds, t._ms, t._us);
}


int to_iso(db2_time_t _time, char* buf)
{
    db2_time_internal_t t = *((db2_time_internal_t*)&_time);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", t._year, t._month, t._day, t._hours, t._minutes, t._seconds, t._ms);

    return 0;
}


#include <stdio.h>

#include "db2_time.h"


#pragma pack(1)
// the motivation is to represent time in microseconds using 64 bits
// and enable native arithmetic operationsm resulting in a time span
typedef struct
{
    unsigned us: 10;
    unsigned ms: 10;
    unsigned seconds: 6;
    unsigned minutes: 6;
    unsigned hours: 5;
    unsigned day: 5;
    unsigned month: 4;
    unsigned year: 14; // 10 bits for 1024 years from 1970 could be something
} db2_time_internal_t;
#pragma pack()


db2_time_t from_time_t(time_t mr_t)
{
    struct tm* parts = localtime(&mr_t);

    db2_time_internal_t res = { 
        .year = parts->tm_year + 1900,
        .month = parts->tm_mon + 1,
        .day = parts->tm_mday,
        .hours = parts->tm_hour,
        .minutes = parts->tm_min,
        .seconds = parts->tm_sec,
        .ms = 0,
        .us = 0
    };

    return *((uint64_t*)&res);
}

db2_time_t from_timespec(struct timespec* spec)
{
    time_t secs = spec->tv_sec;
    struct tm parts;
    
    gmtime_r(&secs, &parts);

    db2_time_internal_t res = { 
        .year = parts.tm_year + 1900,
        .month = parts.tm_mon + 1,
        .day = parts.tm_mday,
        .hours = parts.tm_hour,
        .minutes = parts.tm_min,
        .seconds = parts.tm_sec,
        .ms = spec->tv_nsec / 1000000,
        .us = spec->tv_nsec / 1000
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
    printf("%d-%d-%d %d:%d:%d.%d.%d\n", t.year, t.month, t.day, t.hours, t.minutes, t.seconds, t.ms, t.us);
}


int to_iso(db2_time_t _time, char* buf)
{
    db2_time_internal_t t = *((db2_time_internal_t*)&_time);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", t.year, t.month, t.day, t.hours, t.minutes, t.seconds, t.ms);

    return 0;
}


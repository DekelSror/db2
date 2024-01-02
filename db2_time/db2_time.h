#ifndef db2_time_h
#define db2_time_h

#include <stdint.h>
#include <time.h>

typedef uint64_t db2_time_t;

db2_time_t db2_now(void);
db2_time_t from_time_t(time_t mr_t);
db2_time_t from_timespec(struct timespec* spec);
int to_iso(db2_time_t _time, char* buf);
void time_print_all(db2_time_t dt);


#endif // db2_time_h
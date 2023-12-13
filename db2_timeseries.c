#include <string.h>
#include <sys/socket.h>

#include "utilities.h"
#include "db2_mempool.h"
#include "db2.h"
#include "db2_timeseries.h"


typedef struct
{
    db_value_t* _key;
    unsigned _size;
    timeseries_entry_t _entries[db2_max_timeseries_entries];
} timeseries_t;

struct db2_ts_range_params_t
{
    uint32_t _start_index;
    uint32_t _end_index;
};

static struct db2_ts_range_params_t range_params(db2_ts_descriptor_t ts, db2_time_t start, db2_time_t end);
static double ts_sum_params(db2_ts_descriptor_t ts, struct db2_ts_range_params_t params);

struct numeric_ts_agg_t
{
    char* _name;
    unsigned _num_entries;
    time_t _start;
    time_t _end;
    double _min;
    double _max;
    double _sum;
    double _avg;
    double _median
};


static timeseries_t db[db2_max_timeseries] = { 0 };
static int next_series_index = 0;


int timeseries_create(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };
    struct db_op_ts_create_t header = op->_header._ts_create;

    if (next_series_index == db2_max_timeseries)
    {
        response._status = 500;
        outl("already have %d timeseries", db2_max_timeseries);
        send_response(client_socket, &response);
        return 1;
    }

    db_value_t* key = Mempool.allocate(header._key_size + sizeof(db_value_t));
    key->_size = header._key_size;

    send_response(client_socket, &response);
    stream_in(client_socket, key->_val, header._key_size);

    for (int i = 0; i < next_series_index; i++)
    {
        if (memcmp(key->_val, db[i]._key, header._key_size) == 0)
        {
            Mempool.free(key);
            outl("series '%s' already exists", db[i]._key->_val);
            response._body_size = i;
            send_response(client_socket, &response);

            return 0;
        }
    }
    
    db[next_series_index]._key = key;

    outl("creating timeseries '%s' index %d", key->_val, next_series_index);

    db[next_series_index]._size = 0;

    response._body_size = next_series_index; // dark side of the HACK!!!
    send_response(client_socket, &response);
    next_series_index++;

    return 0;
}

int timeseries_add(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };
    struct db_op_ts_add_t header = op->_header._ts_add;
    timeseries_t* ts = db + header._ts;

    db2_time_t add_time = db2_now();

    int can_add = header._ts < next_series_index; 
    can_add = can_add && db[header._ts]._size < db2_max_timeseries_entries;

    if (can_add)
    {
        ts->_entries[ts->_size]._time = add_time;
        ts->_entries[ts->_size]._val = header._val;
        ts->_size++;
    } 
    else
    {
        response._status = 500;
    }

    send_response(client_socket, &response);

    return !(response._status == 200);
}

db2_time_t timeseries_end(db_op_t* op)
{
    struct db_op_ts_add_t header = op->_header._ts_add;

    return db[header._ts]._entries[0]._time;   
}

db2_time_t timeseries_end(db_op_t* op)
{
    struct db_op_ts_add_t header = op->_header._ts_add;

    return db[header._ts]._entries[db[header._ts]._size - 1]._time;   
}

int timeseries_get_range(db_op_t* op, int client_socket)
{
    db_response_t response = { ._status = 200 };

    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    struct db2_ts_range_params_t params = range_params(header._ts, header._start, header._end);

    if (params._end_index == params._start_index)
    {
        response._status = 404;
        send_response(client_socket, &response);
    }

    response._body_size = (params._end_index - params._start_index) * sizeof(timeseries_entry_t);
    send_response(client_socket, &response);
    
    ssize_t sent = stream_out(client_socket, db[header._ts]._entries + params._start_index, response._body_size);
    response._body_size = sent;
    
    send_response(client_socket, &response);
}


int timeseries_sum(db_op_t* op, int client_socket)
{
    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    struct db2_ts_range_params_t params = range_params(header._ts, header._start, header._end);

    return ts_sum_params(header._ts, params);
}

static double ts_sum_params(db2_ts_descriptor_t ts, struct db2_ts_range_params_t params)
{
    double sum = 0.0;
    for (uint32_t i = params._start_index; i < params._end_index + 1; i++)
    {
        sum += db[ts]._entries[i]._val;
    }

    return sum;
}


int db2_numeric_ts_avg(db_op_t* op, int client_socket)
{
    struct db_op_ts_get_range_t header = op->_header._ts_get_range;
    struct db2_ts_range_params_t params = range_params(header._ts, header._start, header._end);
    
    return ts_sum_params(header._ts, params) / (params._end_index - params._start_index);
}


double max(void)
{
    return 0.0;
}


double min(void)
{
    return 0.0;
}

int map()
{

}

// ._start_index == ._end_index on result means failure for now
static struct db2_ts_range_params_t range_params(db2_ts_descriptor_t ts, db2_time_t start, db2_time_t end)
{
    timeseries_t series = db[ts];

    struct db2_ts_range_params_t res = { 0 };
    if (start > series._entries[series._size - 1]._time)
    {
        return res;
    }

    uint32_t left = 0;
    uint32_t right = series._size;

    while (left < right)
    {
        uint32_t mid = left + (right - left) / 2;
        uint64_t entry_time = series._entries[mid]._time;
        if (entry_time >= start)
        {
            res._start_index = mid;
            break;
        }
        else
        {
            left = mid + 1;
        }
    }

    left = res._start_index;

    while (left < right)
    {
        uint32_t mid = left + (right - left) / 2;
        uint64_t entry_time = series._entries[mid]._time;
        
        if (entry_time >= end)
        {
            res._end_index = mid;
            break;
        }
        else
        {
            right = mid - 1;
        }
    }

    return res;
}
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "db2_mempool.h" // Mempool
#include "utilities.h" // stream_in stream_out
#include "db2_client.h"


static uint64_t simple_hash(char *key, uint32_t len);
uint64_t (*db_hash)(char *, uint32_t) = simple_hash;

static int client_socket = -1;

static db_response_t response = { 0 };
static ssize_t send_op(db_op_t* op)
{
    return send(client_socket, op, sizeof(db_op_t), 0);
}

static ssize_t recieve_response(void)
{
    return recv(client_socket, &response, sizeof(db_response_t), 0);
}

static int db2_connect(void)
{

    struct sockaddr_un server_addr = { 
        .sun_family = AF_UNIX,
        .sun_path = db2_socket_path
    };

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    connect(client_socket, (const struct sockaddr*)&server_addr, sizeof(server_addr));

    recieve_response();

    return response.status;
}

static void db2_stop(void)
{
    close(client_socket);
    client_socket = -1;
}

// kv


static int db2_insert(char* key, uint32_t key_len, void* val, uint32_t val_len)
{
    db_op_t op = {
        .op = Db2OpTypes.insert,
        .header.insert.key_size = key_len,
        .header.insert.val_size = val_len,
        .header.insert.key_hash = db_hash(key, key_len)
    };

    outl("client insert, key '%s' (len %u), hash %lu", key, key_len, db_hash(key, key_len));

    send_op(&op);
    recieve_response(&response);

    if (response.status == 200)
    {
        stream_out(client_socket, key, key_len);
        stream_out(client_socket, val, val_len);

        recieve_response();
        
    }

    return response.status;
}

static int db2_remove(char* key, uint32_t key_len)
{
    db_op_t op = {
        .op = Db2OpTypes.remove,
        .header.remove.key_hash = db_hash(key, key_len),
    };

    send_op(&op);
    recieve_response();

    return response.status;
}

static void* db2_find(char* key, uint32_t key_len)
{
    db_op_t op = {
        .op = Db2OpTypes.find,
        .header.find.key_hash = db_hash(key, key_len)
    };

    outl("client find, key '%s' (len %u), hash %lu", key, key_len, db_hash(key, key_len));

    send_op(&op);
    recieve_response();
    void* found = NULL;

    if (response.status == 200)
    {
        found = Mempool.allocate(response.body_size);
        stream_in(client_socket, found, response.body_size);
    }

    return found;
}


// timeseries

static db2_ts_descriptor_t db2_timeseries_create(char* name, unsigned name_len)
{
    db_op_t op = {
        .op = Db2OpTypes.ts_create,
        .header.ts_create.key_size = name_len
    };
    
    send_op(&op);
    recieve_response();

    struct db_ts_create_response* ts_response = (struct db_ts_create_response*)&response;

    if (ts_response->status == 200) // can create timeseries
    {
        stream_out(client_socket, name, name_len);
        recieve_response((db_response_t*)(&response));

        if (ts_response->status == 200)
        {
            return ts_response->ts;
        }
    }
    
    return -1;
}

static int db2_timeseries_add(db2_ts_descriptor_t ts, double val)
{
    db_op_t op = {
        .op = Db2OpTypes.ts_add,
        .header.ts_add.ts = ts,
        .header.ts_add.val = val
    };

    send_op(&op);
    recieve_response();
    
    return response.status;
}

static db2_time_t db2_timeseries_start_end(db2_ts_descriptor_t ts, int type)
{
    struct {
        int status;
        db2_time_t time;
    } ts_response = { 0 };

    db_op_t op = {
        .op = Db2OpTypes.ts_start_end,
        .header.ts_start_end.ts = ts,
        .header.ts_start_end.type = type
    };

    send_op(&op);
    recv(client_socket, &ts_response, sizeof(ts_response), 0);

    if (ts_response.status == 200)
    {
        return ts_response.time;
    }

    outl("did not get time for %d %d", ts, type);
    
    return -1;
}

static db2_time_t db2_timeseries_start(db2_ts_descriptor_t ts)
{
    return db2_timeseries_start_end(ts, 0);
}

static db2_time_t db2_timeseries_end(db2_ts_descriptor_t ts)
{
    return db2_timeseries_start_end(ts, 1);
    
}

static timeseries_entry_t* db2_timeseries_get_range(db2_ts_descriptor_t ts, db2_time_t start, db2_time_t end)
{
    db_op_t op = {    
        .op = Db2OpTypes.ts_get_range,
        .header.ts_get_range.ts = ts,
        .header.ts_get_range.start = start,
        .header.ts_get_range.end = end
    };

    send_op(&op);
    recieve_response();

    void* result = NULL;

    outl("client ts_get_range initial response %d %d", response.status, response.body_size);

    if (response.status == 200)
    {
        int result_size = response.body_size;
        result = Mempool.allocate(response.body_size);
        outl("get_range streaming in the result");
        stream_in(client_socket, result, response.body_size);
        outl("get_range got result; waiting for response from server");
        recieve_response(&response);

        if (response.body_size != result_size)
        {
            outl("no good result size no good had %d got %d", result_size, response.body_size);
        }
    }

    return result;
}

double db2_nts_sum(db2_ts_descriptor_t ts, time_t start, time_t end)
{
    db_op_t op = {
        .op = Db2OpTypes.ts_get_range,
        .header.ts_get_range.ts = ts,
        .header.ts_get_range.start = start,
        .header.ts_get_range.end = end
    };

    send_op(&op);
    recieve_response();

    double res = 0.0;
    if (response.status == 200)
    {
        stream_in(client_socket, (char*)(&res), sizeof(res));
    }

    return res;
}


double db2_nts_avg(db2_ts_descriptor_t ts, time_t start, time_t end)
{
    db_op_t op = {
        .op = Db2OpTypes.ts_get_range,
        .header.ts_get_range.ts = ts,
        .header.ts_get_range.start = start,
        .header.ts_get_range.end = end
    };

    send_op(&op);
    recieve_response();

    double res = 0.0;
    if (response.status == 200)
    {
        stream_in(client_socket, (char*)(&res), sizeof(res));

    }

    return res;
}

const struct db2_client_t Db2 = {
    .client = {
        .connect = db2_connect,
        .stop = db2_stop,
    },
    .kv = {
        .find = db2_find,
        .insert = db2_insert,
        .remove = db2_remove,

    },
    .timeseries = {
        .create = db2_timeseries_create,
        .add = db2_timeseries_add,
        .get_range = db2_timeseries_get_range,
        .start = db2_timeseries_start,
        .end = db2_timeseries_end
    }
};

static uint64_t simple_hash(char *key, uint32_t _len)
{
    int len = (int)_len;
    uint64_t h = 5381;

    for (int i = 0; i < len; i++)
    {
        h = (h * 33) + key[i];
    }

    return h;
}

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


#include "db2_mempool.h"
#include "utilities.h"
#include "db2_types.h"
#include "db2_client.h"

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
static uint64_t simple_hash(char *key, uint32_t len);
uint64_t (*db_hash)(char *, uint32_t) = simple_hash;

static int client_socket = -1;

static ssize_t send_op(db_op_t* op)
{
    return send(client_socket, op, sizeof(db_op_t), 0);
}

static ssize_t recieve_response(db_response_t* response)
{
    return recv(client_socket, response, sizeof(db_response_t), 0);
}


static int db2_connect(void)
{
    db_response_t response = { 0 };

    struct sockaddr_un server_addr = { 
        .sun_family = AF_UNIX,
        .sun_path = "some-default-path"
    };

    int config = open("../db2_config", O_RDONLY);
    ssize_t path_read = read(config, server_addr.sun_path, 108);
    (void)path_read;
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    connect(client_socket, (const struct sockaddr*)&server_addr, sizeof(server_addr));

    ssize_t recvd = recieve_response(&response);
    (void)recvd;

    return response._status;
}

static int db2_message(const char* msg, size_t msg_len)
{
    db_response_t response = { 0 };
    ssize_t sent =  send(client_socket, msg, msg_len, 0);
    outl("got %ld for msg '%s'", sent, msg);

    ssize_t recvd = recieve_response(&response);
    (void)recvd;

    return response._status;
}

static void db2_stop(void)
{
    close(client_socket);
}

// kv


static int db2_insert(char* key, uint32_t key_len, void* val, uint32_t val_len)
{
    db_response_t response = { 0 };
    
    outl("insert key '%s' start", key);

    db_op_t op = {
        ._op = op_insert,
        ._header._insert._key_size = key_len,
        ._header._insert._val_size = val_len
    };

    send_op(&op);
    recieve_response(&response);
    outl("got ack for insert; starting to send key/value");

    stream_out(client_socket, key, key_len);
    stream_out(client_socket, val, val_len);

    ssize_t recvd = recieve_response(&response);
    (void)recvd;
    
    outl("insert status %d", response._status);

    return response._status;
}

static int db2_remove(char* key, uint32_t key_len)
{
    db_response_t response = { 0 };
    db_op_t op = {
        ._op = op_remove,
        ._header._remove._key_hash = db_hash(key, key_len),
    };

    ssize_t sent = send_op(&op);
    (void)sent;

    ssize_t recvd = recieve_response(&response);
    (void)recvd;

    return response._status;
}

static void* db2_find(char* key, uint32_t key_len)
{
    db_op_t op = {
        ._op = op_find,
        ._header._find._key_hash = db_hash(key, key_len)
    };

    db_response_t response = { 0 };

    ssize_t sent = send_op(&op);
    ssize_t recvd = recieve_response(&response);
    
    db_value_t* found = Mempool.allocate(response._body_size);
    stream_in(client_socket, found, response._body_size);

    // recieve final ack?`
    return found->_val;
}


// timeseries

static db2_ts_descriptor_t db2_timeseries_create(char* name, unsigned name_len)
{
    db_response_t response = { 0 };

    db_op_t op = {
        ._op = op_ts_create,
        ._header._ts_create._key_size = name_len
    };
    

    ssize_t sent = send_op(&op);

    ssize_t recvd = recieve_response(&response);
    (void)sent;
    (void)recvd;


    stream_out(client_socket, name, name_len);

    db2_ts_descriptor_t ts = -1;

    recv(client_socket, &ts, sizeof(ts), 0);
    
    return ts;
}

static int db2_timeseries_add(db2_ts_descriptor_t ts, void* val, uint32_t val_len)
{
    db_response_t response = { 0 };
    db_op_t op = {
        ._op = op_ts_add,
        ._header._ts_add._val_size = val_len,
        ._header._ts_add._ts = ts,
    };
    
    
    ssize_t sent = send_op(&op);

    ssize_t recvd = recieve_response(&response);
    (void)sent;
    (void)recvd;

    stream_out(client_socket, val, val_len);

    return response._status;
}

static int db2_timeseries_get_range(db2_ts_descriptor_t ts, time_t start, time_t end, void* buf)
{
    db_response_t response = { 0 };
    db_op_t op = {    
        ._op = op_ts_get_range,
        ._header._ts_get_range._ts = ts,
        ._header._ts_get_range._start = start,
        ._header._ts_get_range._end = end
    };

    ssize_t sent = send_op(&op);
    
    ssize_t recvd = recieve_response(&response);
    (void)sent;
    (void)recvd;
    if (response._status == 200)
    {
        // db_value_t* val = (db_value_t*)response._body;
        // memmove(buf, val->_val, val->_size);
    }

    return response._status;
}

const db2_client_t Db2 = {
    .connect = db2_connect,
    .stop = db2_stop,
    .find = db2_find,
    .insert = db2_insert,
    .remove = db2_remove,
    .message = db2_message,
    .timeseries_create = db2_timeseries_create,
    .timeseries_add = db2_timeseries_add,
    .timeseries_get_range = db2_timeseries_get_range
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
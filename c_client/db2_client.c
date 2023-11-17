
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "db2_types.h"
#include "db2_client.h"

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

static int client_socket = -1;

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

    ssize_t recvd = recv(client_socket, &response, 4, 0);
    (void)recvd;

    return response._status;
}

static int db2_message(const char* msg, size_t msg_len)
{
    db_response_t response = { 0 };
    ssize_t sent =  send(client_socket, msg, msg_len, 0);
    outl("got %ld for msg '%s'", sent, msg);

    ssize_t recvd = recv(client_socket, &response, 4, 0);
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
    char op_buf[sizeof(db_op_t) + 0x4000];
    
    outl("insert key '%s' start", key);

    db_op_t* op = (db_op_t*)op_buf;
    op->_op = op_insert;
    op->_header._insert._key_size = key_len;
    op->_header._insert._val_size = val_len;
    op->_size = sizeof(db_op_t) + key_len + val_len;
    memmove(op->_body, key, key_len);
    memmove(op->_body + key_len, val, val_len);

    ssize_t sent = send(client_socket, op, op->_size, 0);
    ssize_t recvd = recv(client_socket, &response, 4, 0);
    (void)sent;
    (void)recvd;
    
    outl("insert status %d", response._status);

    return response._status;
}

static int db2_remove(char* key, uint32_t key_len)
{
    db_response_t response = { 0 };
    char op_buf[sizeof(db_op_t) + 0x4000];
    db_op_t* op = (db_op_t*)op_buf;
    
    op->_op = op_remove,
    op->_header._remove._key_size = key_len;
    op->_size = sizeof(db_op_t) + key_len;

    memmove(op->_body, key, key_len);

    ssize_t sent = send(client_socket, op, sizeof(db_op_t) + key_len, 0);
    (void)sent;

    ssize_t recvd = recv(client_socket, &response, 4, 0);
    (void)recvd;

    return response._status;
}



static char response_buf[0x4000] = { 0 };
static void* db2_find(char* key, uint32_t key_len)
{
    char op_buf[sizeof(db_op_t) + 0x4000] = { 0 };

    db_op_t* op = (db_op_t*)op_buf;
    db_response_t* response = (db_response_t*)response_buf;
    
    op->_op = op_find,
    op->_header._find._key_size = key_len;

    op->_size = sizeof(db_op_t) + key_len;

    memmove(op->_body, key, key_len);

    ssize_t sent = send(client_socket, op, op->_size, 0);

    outl("sent find '%s' got %ld", key, sent);

    ssize_t recvd = recv(client_socket, response, 0x4000, 0);
    outl("recvd %ld for find of '%s'", recvd, key);

    return response->_status == 200 ? response->_body : NULL;
}


// timeseries

static db2_ts_descriptor_t db2_timeseries_create(char* name, unsigned name_len)
{
    char res_buf[sizeof(db_response_t) + 4] = { 0 };
    db_response_t* response = (db_response_t*)res_buf;

    char op_buf[1024] = { 0 };
    db_op_t* op = (db_op_t*)op_buf;
    op->_op = op_ts_create;
    op->_header._ts_create._key_size = name_len;
    memmove(op->_body, name, name_len);

    op->_size = sizeof(db_op_t) + name_len;

    ssize_t sent = send(client_socket, op, op->_size, 0);

    ssize_t recvd = recv(client_socket, response, 8, 0);
    (void)sent;
    (void)recvd;

    struct ts_create_res_t 
    {
        int _ts;
    };

    return ((struct ts_create_res_t*)response->_body)->_ts;
}

static int db2_timeseries_add(db2_ts_descriptor_t ts, void* val, uint32_t val_len)
{
    db_response_t response = { 0 };
    char op_buf[1024] = { 0 };
    db_op_t* op = (db_op_t*)op_buf;

    op->_op = op_ts_add;

    op->_header._ts_add._val_size = val_len;
    op->_header._ts_add._ts = ts;
    op->_size = sizeof(db_op_t) + val_len;
    memmove(op->_body, val, val_len);

    ssize_t sent = send(client_socket, op, op->_size, 0);

    ssize_t recvd = recv(client_socket ,&response, 4, 0);
    (void)sent;
    (void)recvd;

    return response._status;
}

static int db2_timeseries_get_range(db2_ts_descriptor_t ts, time_t start, time_t end, void* buf)
{
    char response_buf[0x4000] = { 0 };
    db_response_t* response = (db_response_t*)response_buf;
    char op_buf[1024] = { 0 };
    db_op_t* op = (db_op_t*)op_buf;
    
    op->_op = op_ts_get_range;
    op->_header._ts_get_range._ts =ts;
    memmove(op->_body, &start, sizeof(time_t));
    memmove(op->_body + 8, &end, sizeof(time_t));

    ssize_t sent = send(client_socket, op, sizeof(db_op_t) + 16, 0);
    
    ssize_t recvd = recv(client_socket, response, 0x4000, 0);
    (void)sent;
    (void)recvd;
    if (response->_status == 200)
    {
        db_value_t* val = (db_value_t*)response->_body;
        memmove(buf, val->_val, val->_size);
    }

    return response->_status;
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
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "db2.h"

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define max_clients (10)

int handle_op(db_op_t* op, db_response_t* response);

static int server_socket = -1;

static const size_t cmd_size = 0x4000;

static struct pollfd clients[max_clients] = { 0 };


int next_client_index(void)
{
    // start at 1 beacause of server pollfd
    for (int i = 1; i < max_clients; i++)
    {
        if (clients[i].fd == -1)
        {
            return i;
        }
    }

    return -1;
}

void remove_client(int index)
{
    if (index > 0 && clients[index].fd != -1)
    {
        close(clients[index].fd);
        clients[index].events = 0;
        clients[index].fd = -1;
    }
}

#include <fcntl.h>
int main(void)
{
    // setup
    // 
    struct sockaddr_un server_addr = 
    {
        .sun_family = AF_UNIX,
        .sun_path = "some-default-path",
    };

    int config = open("./db2_config", O_RDONLY);
    ssize_t path_read = read(config, server_addr.sun_path, 108);

    unlink(server_addr.sun_path); // failure does not matter here

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    int bind_res = bind(server_socket, (const struct sockaddr*)&server_addr, sizeof(server_addr));
    int listen_res = listen(server_socket, 1);

    outl("listening! sock %d bind %d listen %d", server_socket, bind_res, listen_res);

    clients[0].fd = server_socket;
    clients[0].events = POLLIN;

    for (int i = 1; i < max_clients; i++)
    {
        clients[i].fd = -1;
        clients[i].events = 0; // proper setup when connecting
    }

    // repl
    while (1)
    {
        outl("polling...");
        int poll_res = poll(clients, max_clients, -1);

        if (poll_res == -1)
        {
            outl("poll no good!");
            break;
        }

// new client
        if (clients[0].revents & POLLIN)
        {
            outl("new connection request!");

            int client_socket = accept(server_socket, NULL, NULL);
            int index = next_client_index();

            if (index == -1)
            {
                outl("cannot accept any more clients!");
                db_response_t response = {._status = 500};
                send(client_socket, &response, sizeof(db_response_t), 0);
                close(client_socket);
                continue;
            }
            else 
            {
                clients[index].fd = client_socket;
                clients[index].events = POLLIN | POLLHUP;
                
                db_response_t response = {._status = 200};
                send(clients[index].fd, &response, sizeof(db_response_t), 0);
                
                
                outl("accepted new connection %d at %d", client_socket, index);
            }
        }

// ops
        char raw_ops[0x4000] = { 0 };
        char response_buf[0x4000] = { 0 };
        db_response_t* response = (db_response_t*)response_buf;

        for (int i = 1; i < max_clients; i++)
        {
            if (clients[i].revents & POLLIN)
            {
                outl("pollin on %d", clients[i].fd);
                
                memset(raw_ops, 0, sizeof(raw_ops));
                ssize_t ops_length = recv(clients[i].fd, raw_ops, cmd_size, 0);


                if (ops_length == 0) // client disconnected (should test if always the case)
                {
                    outl("remove from pollin client %d", i);
                    remove_client(i);
                }

                outl("received %ld bytes from %d", ops_length, i);

                db_op_t* op = (db_op_t*)raw_ops;

                while ((char*)op < raw_ops + ops_length)
                {
                    const char* op_names[] = {"kv_insert", "kv_find", "kv_remove", "ts_create", "ts_add", "ts_get_range"};
                    outl("handling op %s of size %u", op_names[op->_op], op->_size);
                    int body_size = handle_op(op, response);
                    send(clients[i].fd, response, sizeof(db_response_t) + body_size, 0);

                    char* next = (char*)op;
                    next += op->_size;
                    op = (db_op_t*)next;
                }
            }

            if (clients[i].revents & POLLHUP)
            {
                outl("remove from pollhup client %d", i);
                remove_client(i);
            }
        }
    }

    return 0;
}

int handle_op(db_op_t* op, db_response_t* response)
{
    int response_body_size = 0;
    switch (op->_op)
    {
    case op_insert: {
        int res = handle_insert(op);
        response->_status = (res == 0 ? 200 : 400);
        break;
    }
    case op_remove: {
        int res = handle_remove(op);
        response->_status = (res == 0 ? 200 : 400);
        break;
    }
    case op_find: {
        db_value_t* res = handle_find(op);

        if (res != NULL)
        {
            response->_status = 200;
            memmove(response->_body, res->_val, res->_size);
            response_body_size += res->_size;
        }
        else 
        {
            response->_status = 404;
        }
        break;
    }
    case op_ts_create: {
        int ts = timeseries_create(op);
        response_body_size += 4;
        response->_status = (ts != -1 ? 200 : 400);
        break;
    }
    case op_ts_add: {
        int res = timeseries_add(op);
        response->_status = (res == 0 ? 200 : 400);
        break;
    }
    case op_ts_get_range: {
        int res = timeseries_get_range(op);
        response->_status = (res == 0 ? 200 : 400);
        break;
    }

    default:
        outl("badly formatted op");
        // badly formatted op
        break;
    }

    return response_body_size;
}
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#include "db2.h"

#define outl(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define max_clients (10)

static int handle_op(db_op_t* op, int client_index);
static int handle_client_request(int client);
static int setup(void);
static int next_client_index(void);
static void remove_client(int index);
static int handle_connection(void);

static int server_socket = -1;
static struct pollfd clients[max_clients] = { 0 };

static const char* op_names[] = {"kv_insert", "kv_find", "kv_remove", "ts_create", "ts_add", "ts_get_range"};


int main(void)
{
    setup();

    while (1)
    {
        outl("polling...");
        int poll_res = poll(clients, max_clients, -1);

        if (poll_res == -1)
        {
            outl("poll no good!");
            break;
        }

        if (clients[0].revents & POLLIN)
        {
            outl("new connection request!");
            handle_connection();
        }

        for (int i = 1; i < max_clients; i++)
        {
            if (clients[i].revents & POLLIN)
            {
                outl("pollin on %d", clients[i].fd);
                handle_client_request(i);
                
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

static int handle_connection(void)
{
    int client_socket = accept(server_socket, NULL, NULL);
    int index = next_client_index();

    if (index == -1)
    {
        outl("cannot accept any more clients!");
        db_response_t response = {._status = 500};
        send(client_socket, &response, sizeof(db_response_t), 0);
        close(client_socket);

        return 1;
    }
    else 
    {
        clients[index].fd = client_socket;
        clients[index].events = POLLIN | POLLHUP;
        
        db_response_t response = {._status = 200};
        send(clients[index].fd, &response, sizeof(db_response_t), 0);
        
        outl("accepted new connection %d at %d", client_socket, index);

        return 0;
    }
}

static int handle_client_request(int client)
{
    db_op_t op = { 0 };
    ssize_t ops_length = recv(clients[client].fd, &op, sizeof(db_op_t), 0);

    if (ops_length == 0)
    {
        outl("pollin disconnect %d", client);
        remove_client(client);
        return 0;
    }

    outl("recieved %ld bytes from %d", ops_length, client);

    int op_res = handle_op(&op, client);
    (void)op_res;

    return 0;
}

static int handle_op(db_op_t* op, int client_index)
{
    int client_socket = clients[client_index].fd;
    db_response_t response = { 0 };

    switch (op->_op)
    {
    case op_insert: {
        int res = handle_insert(op, client_socket);
        response._status = res ? 500 : 200;
        break;
    }
    case op_remove: {
        handle_remove(op);
        response._status = 200;
        break;
    }
    case op_find: {
        int res = handle_find(op, client_socket);
        response._status = res ? 404 : 200;
        break;
    }
    case op_ts_create: {
        timeseries_create(op, client_socket);
        break;
    }
    case op_ts_add: {
        timeseries_add(op, client_socket);
        break;
    }
    case op_ts_get_range: {
        timeseries_get_range(op, client_socket);
        break;
    }

    default:
        outl("badly formatted op %d", op->_op);
        response._status = 500;
        break;
    }

    send(client_socket, &response, sizeof(db_response_t), 0);
    return 0;
}

static int next_client_index(void)
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

static void remove_client(int index)
{
    if (index > 0 && clients[index].fd != -1)
    {
        close(clients[index].fd);
        clients[index].events = 0;
        clients[index].fd = -1;
    }
}

static int setup(void)
{
    struct sockaddr_un server_addr = 
    {
        .sun_family = AF_UNIX,
        .sun_path = "some-default-path",
    };

    int config = open("./db2_config", O_RDONLY);
    ssize_t path_read = read(config, server_addr.sun_path, 108);
    (void)path_read;

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

    return 0;
}



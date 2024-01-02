#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "utilities.h"

int stream_in(int socket, char* buf, ssize_t size)
{
    ssize_t transferred = 0;

    while (transferred < size)
    {
        uint32_t remaining = size - transferred;
        uint32_t chunk_size = remaining < 0x4000 ? remaining : 0x4000;

        recv(socket, buf + transferred, chunk_size, 0);

        transferred += chunk_size;
    }

    return transferred == size;
}

int stream_out(int socket, const char* data, ssize_t size)
{
    ssize_t transferred = 0;

    while (transferred < size)
    {
        uint32_t remaining = size - transferred;
        uint32_t chunk_size = remaining < 0x4000 ? remaining : 0x4000;

        send(socket, data + transferred, chunk_size, 0);

        transferred += chunk_size;
    }

    return transferred == size;
}

ssize_t send_response(int client_socket, db_response_t* response)
{
    return send(client_socket, response, sizeof(db_response_t), 0);
}


#ifndef utilities
#define utilities


#include <unistd.h>
#include "db2_types.h"


int stream_out(int socket, const char* data, ssize_t size);
int stream_in(int socket, char* buf, ssize_t size);

ssize_t send_response(int client_socket, db_response_t* response);


#endif // utilities


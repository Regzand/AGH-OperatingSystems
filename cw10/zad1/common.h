#ifndef COMMON_H
#define COMMON_H

// sockets settings
#define NET_BACKLOG     10
#define LOCAL_BACKLOG   10
#define PROTOCOL        SOCK_STREAM

// server settings
#define MAX_CLIENTS 20

// error handling
void error(char* msg);
void error_msg(char* msg);

#endif

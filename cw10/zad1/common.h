#ifndef COMMON_H
#define COMMON_H

#define NET_BACKLOG 10
#define LOCAL_BACKLOG 10

#define PROTOCOL SOCK_STREAM

#define MAX_CLIENTS 20

// client
typedef struct client {
    char* name;
    int fd;
} client;

// task payload
typedef struct task {
    int id;
    char op;
    int arg1;
    int arg2;
} task;

// answer payload
typedef struct answer {
    int id;
    int ans;
} answer;

// error handling
void error(char* msg);
void error_msg(char* msg);

#endif

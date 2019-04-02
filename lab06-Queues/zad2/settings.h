#ifndef SETTINGS_H
#define SETTINGS_H

// limit of clients per server
#define MAX_CLIENTS       10

// limit of message length
#define MAX_MESSAGE       40

// client queue name size
#define CLIENT_NAME_SIZE  8

// client queue name
#define SERVER_NAME       "/serverqueue"

// size of request struct
#define SIZE_OF_REQUEST   MAX_MESSAGE + 1 + sizeof(int)

// queues setup
#define MQ_MAXMSG         10
#define MQ_MSGSIZE        SIZE_OF_REQUEST

// order codes
#define REQUEST_INIT      1
#define REQUEST_END       2
#define REQUEST_STOP      3
#define REQUEST_MIRROR    10
#define REQUEST_CALC      11
#define REQUEST_TIME      12
#define REQUEST_RESPONSE  20

// message struct
typedef struct request {
    char type;
    int pid;
    char message[MAX_MESSAGE];
} request;

#endif
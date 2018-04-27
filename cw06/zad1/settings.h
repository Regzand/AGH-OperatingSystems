#ifndef SETTINGS_H
#define SETTINGS_H

// limit of clients per server
#define MAX_CLIENTS       10

// limit of message length
#define MAX_MESSAGE       40

// size of request struct
#define SIZE_OF_REQUEST   sizeof(pid_t) + sizeof(char) * MAX_MESSAGE

// number used while calculating server queue key
#define SERVER_KEY_NUMBER 13

// order codes
#define REQUEST_INIT      1             // <queue key>
#define REQUEST_END       2
#define REQUEST_STOP      3
#define REQUEST_MIRROR    10
#define REQUEST_CALC      11
#define REQUEST_TIME      12
#define REQUEST_RESPONSE  20

// message struct
typedef struct request {
    long type;
    pid_t pid;
    char message[MAX_MESSAGE];
} request;

#endif
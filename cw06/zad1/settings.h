#ifndef SETTINGS_H
#define SETTINGS_H

// limit of clients per server
#define MAX_CLIENTS       10

// limit of message length
#define MAX_MESSAGE       20

// number used while calculating server queue key
#define SERVER_KEY_NUMBER 13

// order codes
#define REQUEST_INIT      1
#define REQUEST_END       2
#define REQUEST_STOP      3
#define REQUEST_MIRROR    10
#define REQUEST_CALC      11
#define REQUEST_TIME      12

// message struct
typedef struct message {
    long type;
    char message[MAX_MESSAGE];
} message;

#endif
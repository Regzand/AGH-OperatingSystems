#ifndef COMMON_H
#define COMMON_H



//==================================================
// SETTINGS
//==================================================

// sockets settings
#define NET_BACKLOG         10
#define LOCAL_BACKLOG       10
#define PROTOCOL            SOCK_STREAM

// server settings
#define PINGER_INTERVAL     1
#define PINGER_TIMEOUT      1
#define MAX_CLIENTS         20

// messages settings
#define MAX_CLIENT_NAME     9
#define MAX_DATA            20

// client status
#define STATUS_REGISTERED   1<<0
#define STATUS_PINGED       1<<1

// message types
#define MSG_ACCEPTED        1
#define MSG_REJECTED        2
#define MSG_PING            3
#define MSG_REGISTER        4
#define MSG_TASK            10
#define MSG_RESULT          11

// operations
#define OP_ADD              0
#define OP_SUB              1
#define OP_MUL              2
#define OP_DIV              3



//==================================================
// STRUCTURES
//==================================================

// message payload
typedef struct message {
    int type;
    char data[MAX_DATA];
} message;

// client data
typedef struct client {
    int fd;                         // client socket file descriptor
    char name[MAX_CLIENT_NAME];     // client name
    int status;                     // client status (bit mask)
} client;



//==================================================
// FUNCTIONS
//==================================================

// sending and receiving
void send_message(int fd, int type, const void *data, size_t len);
int read_message(int fd, struct message* msg);
int read_message_type(int fd);

// operations
char* operation_to_string(int op);
int string_to_operation(char* op);
int calculate(int op, int arg1, int arg2);


#endif

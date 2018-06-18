#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "../utils/log.h"
#include "common.h"

pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;

void send_message(int fd, int type, const void *data, size_t len) {

    struct message msg;
    msg.type = type;
    memcpy(msg.data, data, (len > MAX_DATA ? MAX_DATA : len));

    if (write(fd, &msg, sizeof(struct message)) == -1)
        exit_fatal_no("An error occurred while sending message");

}

int read_message(int fd, struct message *msg) {

    int result = read(fd, msg, sizeof(struct message));
    if (result == -1)
        exit_fatal_no("An error occurred while reading message");

    return result;
}

int read_message_type(int fd) {
    struct message msg;
    read_message(fd, &msg);

    return msg.type;
}

char* operation_to_string(int op){

    if(op == OP_ADD)
        return "ADD";
    if(op == OP_SUB)
        return "SUB";
    if(op == OP_MUL)
        return "MUL";
    if(op == OP_DIV)
        return "DIV";
    return NULL;

}

int string_to_operation(char* op){

    if(strcmp(op, "ADD") == 0)
        return OP_ADD;
    if(strcmp(op, "SUB") == 0)
        return OP_SUB;
    if(strcmp(op, "MUL") == 0)
        return OP_MUL;
    if(strcmp(op, "DIV") == 0)
        return OP_DIV;
    return -1;

}

int calculate(int op, int arg1, int arg2){

    if(op == OP_ADD)
        return arg1 + arg2;
    if(op == OP_SUB)
        return arg1 - arg2;
    if(op == OP_MUL)
        return arg1 * arg2;
    if(op == OP_DIV)
        return arg1 / arg2;
    return 0;

}

void logger_lock(void *udata, int lock){
    if(lock)
        pthread_mutex_lock(udata);
    else
        pthread_mutex_unlock(udata);
}

void setup_logger(){
    log_set_udata(&mutex_log);
    log_set_lock(logger_lock);
    log_set_level(LOGGER_LEVEL);
}

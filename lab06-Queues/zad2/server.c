#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>

#include "settings.h"

// server status (1 - running, 0 - stopped)
int server_status = 1;

// server queue
int server_queue_fd = -1;

// number of clients
int clients_count = 0;

// clients
struct client {
    int pid;
    int queue;
};

struct client clients[MAX_CLIENTS];

int get_client_queue(int pid){
    for(int i = 0; i < clients_count; i++)
        if(clients[i].pid == pid)
            return clients[i].queue;
    return -1;
}

void send_response(int pid, char* msg){

    if(msg != NULL && strlen(msg) >= MAX_MESSAGE)
        msg[MAX_MESSAGE-1] = '\0';

    struct request req;
    req.type = REQUEST_RESPONSE;
    req.pid = getpid();
    if(msg != NULL)
        strcpy(req.message, msg);

    // send request
    if(mq_send(get_client_queue(pid), &(req.type), SIZE_OF_REQUEST, 0) == -1){
        perror("An error occurred while sending response");
        exit(1);
    }


}

void handle_request_init(struct request req){

    // get client id
    int client_id = clients_count++;
    if(client_id >= MAX_CLIENTS){
        fprintf(stderr, "Client limit exceeded!\n");
        return;
    }

    // save client info
    clients[client_id].pid = req.pid;

    // open queue
    clients[client_id].queue = mq_open(req.message, O_WRONLY);
    if(clients[client_id].queue == -1){
        perror("An error occurred while accessing client queue");
        exit(1);
    }

    // send response
    send_response(req.pid, "");

    // logging
    printf("Connected to client (pid: %d, queue: %s, fd: %d)\n", clients[client_id].pid, req.message, clients[client_id].queue);

}

void handle_request_end(struct request req){

    // set server status
    server_status = 0;
}

void handle_request_stop(struct request req){

    // disable client
    for(int i = 0; i < clients_count; i++){
        if(clients[i].pid == req.pid){

            if(clients[i].queue != -1){
                if (mq_close(clients[i].queue) == -1) {
                    perror("An error occurred while closing client queue");
                    exit(1);
                }
            }

            clients[i].pid = -1;
            clients[i].queue = -1;
        }
    }
}

void handle_request_mirror(struct request req){

    int len = strlen(req.message);

    char msg[MAX_MESSAGE];

    for(int i = 0; i < len; i++)
        msg[i] = req.message[len-i-1];

    msg[len] = '\0';

    // send back the message
    send_response(req.pid, msg);

}

void handle_request_calc(struct request req) {

    // get tokens
    char *operation = strtok(req.message, " ");
    char *a = strtok(NULL, " ");
    char *b = strtok(NULL, " ");

    if (operation == NULL || a == NULL || b == NULL) {
        send_response(req.pid, "Missing arguments");
        return;
    }

    // parse arguments
    int A = atoi(a);
    int B = atoi(b);

    // calculate
    int ans;

    if (strcmp(operation, "ADD") == 0)
        ans = A + B;

    else if (strcmp(operation, "SUB") == 0)
        ans = A - B;

    else if (strcmp(operation, "MUL") == 0)
        ans = A * B;

    else if (strcmp(operation, "DIV") == 0) {

        if(B == 0){
            send_response(req.pid, "Nope, I'll not crash for you.");
            return;
        }

        ans = A / B;

    }else{
        send_response(req.pid, "Unknown operation");
        return;
    }

    char msg[MAX_MESSAGE];
    sprintf(msg, "%d", ans);
    send_response(req.pid, msg);
}

void handle_request_time(struct request req){

    // gets current time
    time_t current_time = time(NULL);

    // converts time to string in local format
    char* c_time_string = ctime(&current_time);

    // sends response
    send_response(req.pid, c_time_string);

}

void handle_requests(){

    // while server is running
    while(server_status){

        // read next request
        request req;
        if(mq_receive(server_queue_fd, &(req.type), SIZE_OF_REQUEST, NULL) == -1){
            perror("An error occurred while reading request from server queue");
            exit(1);
        }

        printf("[REQUEST] type: %d\tpid: %d\t msg: %s\n", (int) req.type, req.pid, req.message);

        // handle request
        switch(req.type){
            case REQUEST_INIT:
                handle_request_init(req);
                break;

            case REQUEST_END:
                handle_request_end(req);
                break;

            case REQUEST_STOP:
                handle_request_stop(req);
                break;

            case REQUEST_MIRROR:
                handle_request_mirror(req);
                break;

            case REQUEST_CALC:
                handle_request_calc(req);
                break;

            case REQUEST_TIME:
                handle_request_time(req);
                break;

            default:
                printf("Received request of unknown type: %d, pid: %d, msg: %s\n", (int) req.type, req.pid, req.message);
        }

    }

}

void handle_sigint(int sig){
    printf("Received SIGINT - closing server\n");
    exit(0);
}

void handle_exit(){

    // closing clients queues
    for(int i = 0; i < clients_count; i++){
        if(clients[i].queue != -1){
            if (mq_close(clients[i].queue) == -1) {
                perror("An error occurred while closing client queue");
                exit(1);
            }
            clients[i].queue = -1;
        }
    }

    // close server queue
    if(server_queue_fd != -1) {
        if (mq_close(server_queue_fd) == -1) {
            perror("An error occurred while closing server queue");
            exit(1);
        }
    }

    // remove server queue
    if(mq_unlink(SERVER_NAME) == -1){
        perror("An error occurred while removing server queue");
        exit(1);
    }

}

void setup_server_queue(){

    struct mq_attr attr;
    attr.mq_maxmsg = MQ_MAXMSG;
    attr.mq_msgsize = MQ_MSGSIZE;

    // create queue
    server_queue_fd = mq_open(SERVER_NAME, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, &attr);
    if(server_queue_fd == -1){
        perror("An error occurred while creating server queue");
        exit(1);
    }

    // logging
    printf("Created server queue (name: %s, fd: %d)\n", SERVER_NAME, server_queue_fd);

}

void setup_sigint(){

    // sets up SIGINT (Ctrl + C) handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("An error occurred while setting up handler for SIGINT");
        exit(1);
    }

}

void setup_atexit(){

    // sets up function to be called at exit
    if(atexit(handle_exit) != 0){
        perror("An error occurred while setting up atexit function");
        exit(1);
    }

}

int main(){

    // setup
    setup_atexit();
    setup_sigint();
    setup_server_queue();

    // main server loop
    handle_requests();

    return 0;
}
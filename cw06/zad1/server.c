#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include "settings.h"

// server status (1 - running, 0 - stopped)
int server_status = 1;

// server queue id
int server_queue_id = -1;

void handle_request_init(char* request){

    // logging
    printf("Received INIT request\n");

    // TODO
}

void handle_request_end(char* request){

    // logging
    printf("Received END request\n");

    // TODO
}

void handle_request_stop(char* request){

    // logging
    printf("Received STOP request\n");

    // TODO
}

void handle_request_mirror(char* request){

    // logging
    printf("Received MIRROR request\n");

    // TODO
}

void handle_request_calc(char* request){

    // logging
    printf("Received CALC request\n");

    // TODO
}

void handle_request_time(char* request){

    // logging
    printf("Received TIME request\n");

    // TODO
}

void handle_requests(){

    // while server is running
    while(server_status){

        // read next request
        message msg;
        if(msgrcv(server_queue_id, &msg, sizeof(char) * MAX_MESSAGE, 0, 0) == -1){
            perror("An error occurred while reading request from server queue");
            exit(1);
        }

        // handle request
        switch(msg.type){
            case REQUEST_INIT:
                handle_request_init(msg.message);
                break;

            case REQUEST_END:
                handle_request_end(msg.message);
                break;

            case REQUEST_STOP:
                handle_request_stop(msg.message);
                break;

            case REQUEST_MIRROR:
                handle_request_mirror(msg.message);
                break;

            case REQUEST_CALC:
                handle_request_calc(msg.message);
                break;

            case REQUEST_TIME:
                handle_request_time(msg.message);
                break;

            default:
                printf("Received request of unknown type %ld: %s\n", msg.type, msg.message);
        }

    }

}

void handle_sigint(int sig){
    printf("Received SIGINT - closing server\n");
    exit(0);
}

void handle_exit(){

    // if server queue isn't available nothing to do
    if(server_queue_id == -1)
        return;

    // close server queue
    if(msgctl(server_queue_id, IPC_RMID, NULL) != 0){
        perror("An error occurred while closing server queue");
    }

}

void setup_server_queue(){

    // get home path
    char* home = getenv("HOME");

    // get server queue key
    int key = ftok(home, SERVER_KEY_NUMBER);
    if(key == -1){
        perror("An error occurred while creating server queue key");
        exit(1);
    }

    // create queue
    server_queue_id = msgget(key, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR );
    if(server_queue_id == -1){
        perror("An error occurred while creating server queue");
        exit(1);
    }

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
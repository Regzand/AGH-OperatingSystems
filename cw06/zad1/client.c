#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "settings.h"

// server queue id
int server_queue_id = -1;

// client queue id
int client_queue_id = -1;

void send_request(int type, char* msg){

    if(msg != NULL && strlen(msg) >= MAX_MESSAGE)
        msg[MAX_MESSAGE-1] = '\0';

    // create request
    struct request req;
    req.type = type;
    req.pid = getpid();
    if(msg != NULL)
        strcpy(req.message, msg);

    // send request
    if(msgsnd(server_queue_id, &req, SIZE_OF_REQUEST, 0) == -1){
        perror("An error occurred while sending request");
        exit(1);
    }

}

void execute_line(char* line){

    // get request type
    char* type = strtok(line, " \n\t");

    // skip if line is empty
    if(type == NULL)
        return;

    // get type
    int request_type = -1;
    if(strcmp(type, "END") == 0)
        request_type = REQUEST_END;

    else if(strcmp(type, "MIRROR") == 0)
        request_type = REQUEST_MIRROR;

    else if(strcmp(type, "CALC") == 0)
        request_type = REQUEST_CALC;

    else if(strcmp(type, "TIME") == 0)
        request_type = REQUEST_TIME;

    if(request_type == -1){
        fprintf(stderr, "Unknown request type: %s\n", type);
        return;
    }

    // send request
    send_request(request_type, strtok(NULL, "\n"));

    // in case of END just terminate itself
    if(request_type == REQUEST_END){
        server_queue_id = -1;
        exit(0);
    }

    // wait for response
    struct request res;
    if(msgrcv(client_queue_id, &res, SIZE_OF_REQUEST, REQUEST_RESPONSE, 0) == -1){
        perror("An error occurred while reading request from client queue");
        exit(1);
    }

    // print response
    printf("< %s\n", res.message);
}

void execute_input(){

    // for each line
    char* line = NULL;
    size_t line_size = 0;
    while(getline(&line, &line_size, stdin) != -1)
        execute_line(line);

}

void connect(){

    // create REQUEST_INIT
    struct request req;
    req.type = REQUEST_INIT;
    req.pid = getpid();
    sprintf(req.message, "%d", client_queue_id);

    // send request
    if(msgsnd(server_queue_id, &req, SIZE_OF_REQUEST, 0) == -1){
        perror("An error occurred while sending INIT request");
        exit(1);
    }

    // wait for response
    if(msgrcv(client_queue_id, &req, SIZE_OF_REQUEST, REQUEST_RESPONSE, 0) == -1){
        perror("An error occurred while reading request from client queue");
        exit(1);
    }

    // logging
    printf("Connected to server\n");

}

void handle_sigint(int sig){
    printf("Received SIGINT - closing client\n");
    exit(0);
}

void handle_exit(){

    // if server is available send REQUEST_STOP
    if(server_queue_id != -1)
        send_request(REQUEST_STOP, NULL);

    // if client queue isn't available nothing to do
    if(client_queue_id == -1)
        return;

    // close client queue
    if(msgctl(client_queue_id, IPC_RMID, NULL) != 0){
        perror("An error occurred while closing client queue");
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
    server_queue_id = msgget(key, 0);
    if(server_queue_id == -1){
        perror("An error occurred while accessing server queue");
        exit(1);
    }

    // logging
    printf("Connected to server queue (id: %d)\n", server_queue_id);

}

void setup_client_queue(){

    // create queue
    client_queue_id = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);
    if(client_queue_id == -1){
        perror("An error occurred while creating client queue");
        exit(1);
    }

    // logging
    printf("Created client queue (id: %d)\n", client_queue_id);

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

    // logging
    printf("Client started (pid: %d)\n", getpid());

    // setup
    setup_atexit();
    setup_sigint();
    setup_client_queue();
    setup_server_queue();
    connect();

    // sending requests
    execute_input();

    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <fcntl.h>

#include "settings.h"

// server queue id
mqd_t server_queue_fd = -1;

// client queue
char* client_queue_name;
mqd_t client_queue_fd = -1;

char* get_random_name(int len){
    const char letters[] = "abcdefghijklmnopqrstuwxyz";

    char* name = malloc((len+1) * sizeof(char));
    for(int i = 0; i < len; i++)
        name[i] = letters[rand()%25];
    name[0] = '/';
    name[len] = '\0';
    return name;
}

void send_request(char type, char* msg){

    if(msg != NULL && strlen(msg) >= MAX_MESSAGE)
        msg[MAX_MESSAGE-1] = '\0';

    struct request req;
    req.type = type;
    req.pid = getpid();
    if(msg != NULL)
        strcpy(req.message, msg);

    // send request
    if(mq_send(server_queue_fd, &(req.type), SIZE_OF_REQUEST, 0) == -1){
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
    char request_type = -1;
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
        exit(0);
    }

    // wait for response
    struct request res;
    if(mq_receive(client_queue_fd, &(res.type), SIZE_OF_REQUEST, NULL) == -1){
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

    // send request
    send_request(REQUEST_INIT, client_queue_name);

    // read next request
    struct request req;
    if(mq_receive(client_queue_fd, &(req.type), SIZE_OF_REQUEST, NULL) == -1){
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

    // sending REQUEST_STOP
    send_request(REQUEST_STOP, "");

    // close client queue
    if(client_queue_fd != -1) {
        if (mq_close(client_queue_fd) == -1) {
            perror("An error occurred while closing client queue");
            exit(1);
        }
    }

    // remove client queue
    if(mq_unlink(client_queue_name) == -1){
        perror("An error occurred while removing client queue");
        exit(1);
    }

    // close server queue
    if(server_queue_fd != -1) {
        if (mq_close(server_queue_fd) == -1) {
            perror("An error occurred while closing server queue");
            exit(1);
        }
    }

}

void setup_server_queue(){

    // open server queue
    server_queue_fd = mq_open(SERVER_NAME, O_WRONLY);
    if(server_queue_fd == -1){
        perror("An error occurred while accessing server queue");
        exit(1);
    }

    // logging
    printf("Connected to server queue (name: %s, fd: %d)\n", SERVER_NAME, server_queue_fd);

}

void setup_client_queue(){

    // get queue name
    client_queue_name = get_random_name(CLIENT_NAME_SIZE);

    struct mq_attr attr;
    attr.mq_maxmsg = MQ_MAXMSG;
    attr.mq_msgsize = MQ_MSGSIZE;

    // create queue
    client_queue_fd = mq_open(client_queue_name, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, &attr);
    if(client_queue_fd == -1){
        perror("An error occurred while creating client queue");
        exit(1);
    }

    // logging
    printf("Created client queue (name: %s, fd: %d)\n", client_queue_name, client_queue_fd);

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
    srand(getpid() * time(NULL));

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
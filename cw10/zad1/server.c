#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <arpa/inet.h>

#include "common.h"

// tasks counter
int tasks = 0;

// server sockets
int net_socket;
int local_socket;

// program arguments
int port;
char *path;

void *main_pinger() {
    while(1){

        // TODO: Ping clients

    }
}

void *main_server() {
    while(1){

        // TODO: Handle connections

    }
}

void handle_console() {
    while (1) {

        // TODO: Handle console

    }
}

void handle_sigint() {
    printf("Received SIGINT - terminating server\n");
    exit(0);
}

void handle_exit() {

    // close inet socket
    if (shutdown(net_socket, SHUT_RDWR) == -1)
        perror("An error occurred while shutting down net server socket");
    if (close(net_socket) == -1)
        perror("An error occurred while closing net server socket");

    // close local socket
    if (shutdown(local_socket, SHUT_RDWR) == -1)
        perror("An error occurred while shutting down local server socket");
    if (close(local_socket) == -1)
        perror("An error occurred while closing local server socket");

    // remove local socket
    if (unlink(path) == -1)
        perror("An error occurred while un linking local server socket");

}

void setup_pinger_thread() {

    // create thread for pinging clients
    pthread_t tid;
    if (pthread_create(&tid, NULL, main_pinger, NULL) != 0)
        error("An error occurred while creating pinger thread");

}

void setup_server_thread() {

    // create thread for handling incoming messages
    pthread_t tid;
    if (pthread_create(&tid, NULL, main_server, NULL) != 0)
        error("An error occurred while creating server thread");

}

void setup_net_socket() {

    // create socket
    net_socket = socket(AF_INET, PROTOCOL, 0);
    if (net_socket == -1)
        error("An error occurred while creating inet server socket");

    // create address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // bind socket to address
    if (bind(net_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
        error("An error occurred while binding inet server socket");

    // start listening
    if (listen(net_socket, NET_BACKLOG) == -1)
        error("An error occurred while listening on inet server socket");

    // logging
    printf("Listening on port %d\n", port);

}

void setup_local_socket() {

    // create socket
    local_socket = socket(AF_LOCAL, PROTOCOL, 0);
    if (local_socket == -1)
        error("An error occurred while creating local server socket");

    // create address
    struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, path);

    // bind socket to address
    if (bind(local_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        error("An error occurred while binding local server socket");

    // start listening
    if (listen(local_socket, LOCAL_BACKLOG) == -1)
        error("An error occurred while listening on local server socket");

    // logging
    printf("Listening on '%s'\n", path);

}

void setup_atexit() {

    // sets up function to be called at exit
    if (atexit(handle_exit) != 0) {
        perror("An error occurred while setting up atexit function");
        exit(1);
    }

}

void setup_sigint() {

    // sets up SIGINT handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("An error occurred while setting up handler for SIGINT");
        exit(1);
    }

}

void parse_arguments(int argc, char **args) {

    // check arguments count
    if (argc < 3)
        error_msg("Missing arguments!\nUsage: <port> <socket path>");

    // parse arguments
    port = atoi(args[1]);
    path = args[2];

}

int main(int argc, char **args) {

    parse_arguments(argc, args);

    setup_sigint();
    setup_atexit();

    setup_net_socket();
    setup_local_socket();

    setup_server_thread();
    setup_pinger_thread();

    handle_console();

    return 0;
}
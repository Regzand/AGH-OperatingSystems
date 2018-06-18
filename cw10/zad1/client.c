#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "../utils/log.h"
#include "common.h"

// server address
struct sockaddr *socket_address;
int socket_address_length;

// client socket
int socket_fd;

// program arguments
char *name;
int connection_type;
char *address;
int port;

void handle_message_task(const int* data){

    // TODO: Add "DIV _ 0" handling

    // logging
    log_info("Received task #%d: %s %d %d", data[0], operation_to_string(data[1]), data[2], data[3]);

    // calculate result
    int result = calculate(data[1], data[2], data[3]);

    // send result
    int response[] = {data[0], result};
    send_message(socket_fd, MSG_RESULT, response, sizeof(response));

    // logging
    log_info("Sent the result of task #%d: %d", data[0], result);

}

void handle_message_ping(){

    // ping - pong
    send_message(socket_fd, MSG_PING, NULL, 0);

    // logging
    log_trace("Pinged back");

}

void handle_messages() {

    while (1) {

        // wait for message
        struct message msg;
        if(read_message(socket_fd, &msg) == 0)
            exit_info("Server closed - terminating client");

        // handle message
        if(msg.type == MSG_TASK)
            handle_message_task((int*) msg.data);
        else if (msg.type == MSG_PING)
            handle_message_ping();
        else
            log_warn("Received message of unexpected type: %d", msg.type);

    }

}

void handle_sigint() {
    log_debug("Received SIGINT - terminating client");
    exit(0);
}

void handle_exit() {

    // close socket
    if (shutdown(socket_fd, SHUT_RDWR) == -1)
        log_error("An error occurred while shutting down socket");
    if (close(socket_fd) == -1)
        log_error("An error occurred while closing socket");

}

void setup_connection() {

    // create socket
    socket_fd = socket(connection_type, PROTOCOL, 0);
    if (socket_fd == -1)
        exit_fatal("An error occurred while creating socket");

    // connect to server
    if (connect(socket_fd, socket_address, socket_address_length) == -1)
        exit_fatal("An error occurred while connecting to server");

    // logging
    if (connection_type == AF_INET)
        log_info("Connected to %s:%d", address, port);
    else
        log_info("Connected to %s", address);

    // wait for response
    if (read_message_type(socket_fd) != MSG_ACCEPTED)
        exit_fatal("Server rejected connection");

    // register using name
    send_message(socket_fd, MSG_REGISTER, name, strlen(name)+1);

    // wait for response
    if (read_message_type(socket_fd) != MSG_ACCEPTED)
        exit_fatal("Server rejected registration");

    // logging
    log_info("Registered as '%s'", name);

}

void setup_net_address() {

    // create address
    socket_address_length = sizeof(struct sockaddr_in);
    struct sockaddr_in *addr = calloc(1, socket_address_length);

    // set address properties
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    // parse ip
    if (inet_pton(AF_INET, address, &(addr->sin_addr)) != 1)
        exit_fatal("Wrong argument!\nAddress has to be in IPv4 format");

    // cast address
    socket_address = (struct sockaddr *) addr;

}

void setup_local_address() {

    // create address
    socket_address_length = sizeof(struct sockaddr_un);
    struct sockaddr_un *addr = calloc(1, socket_address_length);

    // set address properties
    addr->sun_family = AF_LOCAL;
    strcpy(addr->sun_path, address);

    // cast address
    socket_address = (struct sockaddr *) addr;

}

void setup_address() {

    if (connection_type == AF_INET)
        setup_net_address();
    else
        setup_local_address();

}

void setup_atexit() {

    // sets up function to be called at exit
    if (atexit(handle_exit) != 0)
        exit_fatal_no("An error occurred while setting up atexit function");

}

void setup_sigint() {

    // sets up SIGINT handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        exit_fatal_no("An error occurred while setting up handler for SIGINT");

}

void parse_arguments(int argc, char **args) {

    // check arguments count
    if (argc < 4)
        exit_fatal("Missing arguments!\nUsage: <name> <net|local> <address> [port]");

    // parse arguments
    name = args[1];
    address = args[3];

    // parse type
    if (strcmp(args[2], "net") == 0)
        connection_type = AF_INET;
    else if (strcmp(args[2], "local") == 0)
        connection_type = AF_LOCAL;
    else
        exit_fatal("Wrong argument!\nConnection type has to be 'net' or 'local'");

    // parse port if needed
    if (connection_type == AF_INET) {
        if (argc < 5)
            exit_fatal("Missing argument!\nWhile using net connection type argument 'port' is required");

        port = atoi(args[4]);
    }

}

int main(int argc, char **args) {

    setup_logger();

    parse_arguments(argc, args);

    setup_sigint();
    setup_atexit();

    setup_address();
    setup_connection();

    handle_messages();

    return 0;
}
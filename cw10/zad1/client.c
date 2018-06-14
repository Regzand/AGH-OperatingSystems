#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

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

void handle_requests() {

    while (1) {
        // TODO: handle incoming requests
    }

}

void setup_connection() {

    // create socket
    socket_fd = socket(connection_type, PROTOCOL, 0);
    if (socket_fd == -1)
        error("An error occurred while creating socket");

    // connect to server
    if (connect(socket_fd, socket_address, socket_address_length) == -1)
        error("An error occurred while connecting to server");

    // logging
    if (connection_type == AF_INET)
        printf("Connected to %s:%d\n", address, port);
    else
        printf("Connected to %s\n", address);

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
        error_msg("Wrong argument!\nAddress has to be in IPv4 format");

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

void parse_arguments(int argc, char **args) {

    // check arguments count
    if (argc < 4)
        error_msg("Missing arguments!\nUsage: <name> <net|local> <address> [port]");

    // parse arguments
    name = args[1];
    address = args[3];

    // parse type
    if (strcmp(args[2], "net") == 0)
        connection_type = AF_INET;
    else if (strcmp(args[2], "local") == 0)
        connection_type = AF_LOCAL;
    else
        error_msg("Wrong argument!\nConnection type has to be 'net' or 'local'");

    // parse port if needed
    if (connection_type == AF_INET) {
        if (argc < 5)
            error_msg("Missing argument!\nWhile using net connection type argument 'port' is required");

        port = atoi(args[4]);
    }

}

int main(int argc, char **args) {

    parse_arguments(argc, args);

    setup_address();
    setup_connection();

    handle_requests();

    // TODO: Add at exit?
    return 0;
}
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

#include "../utils/log.h"
#include "common.h"

// TODO: check mutexes

int closed = 0; // FIXME: This might be not needed

//==================================================
// DATA
//==================================================

// mutexes
pthread_mutex_t mutex_data = PTHREAD_MUTEX_INITIALIZER;

// clients data
struct client clients[MAX_CLIENTS];

// tasks counter
int tasks = 0;

// server sockets
int net_socket;
int local_socket;

// program arguments
int port;
char *path;



//==================================================
// UTILITY
//==================================================

void remove_client(int id) {

    // lock access to data
    pthread_mutex_lock(&mutex_data);

    clients[id].fd = -1;
    clients[id].name[0] = '\0';
    clients[id].status = 0;

    // unlock access to data
    pthread_mutex_unlock(&mutex_data);

}

void disconnect_client(int id) {

    // close socket
    shutdown(clients[id].fd, SHUT_RDWR);
    close(clients[id].fd);

    // remove client data
    remove_client(id);

}

int get_free_id() {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd == -1)
            return i;
    return -1;
}

int get_id_by_name(const char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (strcmp(clients[i].name, name) == 0)
            return i;
    return -1;
}

int last_index = 0;

int get_next_client() {
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        int index = (last_index + i) % MAX_CLIENTS;
        if (clients[index].status & STATUS_REGISTERED) {
            last_index = index;
            return index;
        }
    }
    return -1;
}


//==================================================
// CONNECTIONS UTILITY
//==================================================

void handle_message_register(int id, const char *name) {

    // check client status
    if (clients[id].status & STATUS_REGISTERED) {

        // send rejected
        send_message(clients[id].fd, MSG_REJECTED, NULL, 0);

        // logging
        log_warn("Registration from client %d '%s' rejected - client already registered", id, clients[id].name);

        return;
    }

    // check name
    if (strlen(name) == 0) {

        // send rejected
        send_message(clients[id].fd, MSG_REJECTED, NULL, 0);

        // logging
        log_warn("Registration from client %d rejected - empty name", id);

        return;
    }

    // check if name not taken
    if (get_id_by_name(name) != -1) {

        // send rejected
        send_message(clients[id].fd, MSG_REJECTED, NULL, 0);

        // logging
        log_warn("Registration from client %d rejected - name '%s' is already taken", id, name);

        return;
    }

    // set client name and status
    strcpy(clients[id].name, name);
    clients[id].status |= STATUS_REGISTERED;

    // send response
    send_message(clients[id].fd, MSG_ACCEPTED, NULL, 0);

    // logging
    log_info("Client %d registered as '%s'", id, clients[id].name);

}

void handle_message_ping(int id) {

    //clients[id].status &= ~STATUS_PINGED; FIXME: Why this doesn't work?

    int a = clients[id].status;
    int b = STATUS_PINGED;
    clients[id].status = a & ~b;

    // logging
    log_trace("Client '%s' responded to ping", clients[id].name);

}

void handle_message_result(int id, int task, int result) {

    log_info("Received task #%d result: %d", task, result);

}

void accept_net() {

    // incoming connection details
    struct sockaddr_in addr;
    socklen_t addr_length = sizeof(struct sockaddr_in);

    // accept connection
    int fd = accept(net_socket, (struct sockaddr *) &addr, &addr_length);
    if (fd == -1)
        exit_fatal_no("An error occurred while accepting connection on net socket");

    // lock access to data
    pthread_mutex_lock(&mutex_data);

    // get id for this client
    int id = get_free_id();

    // if client limit is exceeded
    if (id == -1) {

        // sending rejected response
        send_message(fd, MSG_REJECTED, NULL, 0);

        // logging
        log_warn("Rejected new connection from %s:%d, due to exceeded limit of clients", inet_ntoa(addr.sin_addr),
               ntohs(addr.sin_port));

    } else {

        // sending accepted response
        send_message(fd, MSG_ACCEPTED, NULL, 0);

        // save client data
        clients[id].fd = fd;

        // logging
        log_debug("New connection from %s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }

    // unlock access to data
    pthread_mutex_unlock(&mutex_data);

}

void accept_local() {

    // accept connection
    int fd = accept(local_socket, NULL, NULL);
    if (fd == -1)
        exit_fatal_no("An error occurred while accepting connection on local socket");

    // lock access to data
    pthread_mutex_lock(&mutex_data);

    // get id for this client
    int id = get_free_id();

    // if client limit is exceeded
    if (id == -1) {

        // sending rejected response
        send_message(fd, MSG_REJECTED, NULL, 0);

        // logging
        log_warn("Rejected new local connection, due to exceeded limit of clients");

        // if we have proper id
    } else {

        // sending accepted response
        send_message(fd, MSG_ACCEPTED, NULL, 0);

        // save client data
        clients[id].fd = fd;

        // logging
        log_debug("New local connection");
    }

    // unlock access to data
    pthread_mutex_unlock(&mutex_data);

}

void handle_message(int id) {

    // read message
    struct message msg;
    read_message(clients[id].fd, &msg);

    // handle message
    if (msg.type == MSG_REGISTER)
        handle_message_register(id, msg.data);
    else if (msg.type == MSG_PING)
        handle_message_ping(id);
    else if (msg.type == MSG_RESULT)
        handle_message_result(id, ((int *) msg.data)[0], ((int *) msg.data)[1]);
    else
        log_warn("Client '%s' send message of unexpected type: %d", clients[id].name, msg.type);

}

void handle_disconnect(int id) {

    // logging
    log_info("Client '%s' disconnected", clients[id].name);

    // remove client
    disconnect_client(id);

}



//==================================================
// HANDLERS
//==================================================

void *handle_pinger() {
    while (1) {

        // wait interval
        sleep(PINGER_INTERVAL);

        // get client
        int id = get_next_client();
        if (id == -1)
            continue;

        // set client status
        clients[id].status |= STATUS_PINGED;


        // send ping
        send_message(clients[id].fd, MSG_PING, NULL, 0);

        // logging
        log_trace("Pinged client '%s'", clients[id].name);
        // wait for pong
        sleep(PINGER_TIMEOUT);

        // check if was pinged
        if (clients[id].status & STATUS_PINGED) {

            // logging
            log_debug("Client '%s' is not responding - disconnected", clients[id].name);

            // remove client
            remove_client(id);

        }

    }
}

void *handle_server() {

    // pool of clients and server sockets
    struct pollfd sockets[MAX_CLIENTS + 2];

    // set server sockets
    sockets[MAX_CLIENTS + 0].fd = net_socket;
    sockets[MAX_CLIENTS + 1].fd = local_socket;
    sockets[MAX_CLIENTS + 0].events = POLLIN;
    sockets[MAX_CLIENTS + 1].events = POLLIN;

    // set pool options for clients
    for (int i = 0; i < MAX_CLIENTS; i++)
        sockets[i].events = POLLIN;

    // handle requests
    while (1) {

        // lock access to data
        pthread_mutex_lock(&mutex_data);

        // update sockets file descriptors and (just in case) clear results
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sockets[i].fd = clients[i].fd;
            sockets[i].revents = 0;
        }

        // unlock access to data
        pthread_mutex_unlock(&mutex_data);

        // wait for new requests
        if (poll(sockets, MAX_CLIENTS + 2, -1) == -1)
            exit_fatal_no("An error occurred while pooling");

        // check net socket
        if (sockets[MAX_CLIENTS + 0].revents && POLLIN && !closed)
            accept_net();

        // check local socket
        if (sockets[MAX_CLIENTS + 1].revents && POLLIN && !closed)
            accept_local();

        // handle errors
        for (int i = 0; i < MAX_CLIENTS; i++)
            if (sockets[i].revents & POLLERR)
                log_error("An error reported by pool on client '%s' (id %d)", clients[i].name, i);

        // handle disconnections
        for (int i = 0; i < MAX_CLIENTS; i++)
            if ((sockets[i].revents & POLLHUP))
                handle_disconnect(i);

        // handle messages
        for (int i = 0; i < MAX_CLIENTS; i++)
            if ((sockets[i].revents & POLLIN) && !(sockets[i].revents & POLLHUP))
                handle_message(i);

    }
}

void handle_console() {
    while (1) {

        // task data
        int data[4];
        data[0] = tasks++;
        data[1] = -1;

        // read task
        char op[5];
        scanf("%s %d %d", op, &data[2], &data[3]);

        // get task type
        data[1] = string_to_operation(op);

        if (data[1] == -1) {
            log_error("Unknown operator: %s", op);
            continue;
        }

        // get client
        int id = get_next_client();

        if (id == -1) {
            log_error("No clients available to perform this task");
            continue;
        }

        // send task
        send_message(clients[id].fd, MSG_TASK, data, sizeof(data));

        // logging
        log_info("Sent task #%d to client '%s': %s %d %d", data[0], clients[id].name, op, data[2], data[3]);

    }
}

void handle_sigint() {
    log_debug("Received SIGINT - terminating server");
    exit(0);
}

void handle_exit() {

    closed = 1;

    // disconnect clients
    for (int i = 0; i < MAX_CLIENTS; i++)
        disconnect_client(i);

    // close inet socket
    if (shutdown(net_socket, SHUT_RDWR) == -1)
        log_error_no("An error occurred while shutting down net server socket");
    if (close(net_socket) == -1)
        log_error_no("An error occurred while closing net server socket");

    // close local socket
    if (shutdown(local_socket, SHUT_RDWR) == -1)
        log_error_no("An error occurred while shutting down local server socket");
    if (close(local_socket) == -1)
        log_error_no("An error occurred while closing local server socket");

    // remove local socket
    if (unlink(path) == -1)
        log_error_no("An error occurred while un linking local server socket");

}



//==================================================
// SETUP
//==================================================

void setup_pinger_thread() {

    // create thread for pinging clients
    pthread_t tid;
    if (pthread_create(&tid, NULL, handle_pinger, NULL) != 0)
        exit_fatal_no("An error occurred while creating pinger thread");

}

void setup_server_thread() {

    // create thread for handling incoming messages
    pthread_t tid;
    if (pthread_create(&tid, NULL, handle_server, NULL) != 0)
        exit_fatal_no("An error occurred while creating server thread");

}

void setup_net_socket() {

    // create socket
    net_socket = socket(AF_INET, PROTOCOL, 0);
    if (net_socket == -1)
        exit_fatal_no("An error occurred while creating inet server socket");

    // create address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // bind socket to address
    if (bind(net_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
        exit_fatal_no("An error occurred while binding inet server socket");

    // start listening
    if (listen(net_socket, NET_BACKLOG) == -1)
        exit_fatal_no("An error occurred while listening on inet server socket");

    // logging
    log_info("Listening on port %d", port);

}

void setup_local_socket() {

    // create socket
    local_socket = socket(AF_LOCAL, PROTOCOL, 0);
    if (local_socket == -1)
        exit_fatal_no("An error occurred while creating local server socket");

    // create address
    struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, path);

    // bind socket to address
    if (bind(local_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        exit_fatal_no("An error occurred while binding local server socket");

    // start listening
    if (listen(local_socket, LOCAL_BACKLOG) == -1)
        exit_fatal_no("An error occurred while listening on local server socket");

    // logging
    log_info("Listening on '%s'", path);

}

void setup_clients() {

    // setup clients data
    for (int i = 0; i < MAX_CLIENTS; i++)
        remove_client(i);

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



//==================================================
// MAIN PROGRAM
//==================================================

void parse_arguments(int argc, char **args) {

    // check arguments count
    if (argc < 3)
        exit_fatal("Missing arguments!\nUsage: <port> <socket path>");

    // parse arguments
    port = atoi(args[1]);
    path = args[2];

}

int main(int argc, char **args) {

    parse_arguments(argc, args);

    setup_sigint();
    setup_atexit();

    setup_clients();

    setup_net_socket();
    setup_local_socket();

    setup_server_thread();
    setup_pinger_thread();

    handle_console();

    return 0;
}
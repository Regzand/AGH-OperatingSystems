#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#include "shop.h"

int shop_semaphore = -1;

void handle_shop(){
    // TODO
}

void cleanup_shop(){
    // TODO
}

void cleanup_semaphores(){

    // remove semaphore
    if(semctl(shop_semaphore, 0, IPC_RMID) == -1)
        perror("An error occurred while removing semaphore");

}

void handle_sigint(int signo){
    printf("Received SIGINT - closing barber shop\n");
    exit(0);
}

void handle_exit(){

    // clean up
    cleanup_semaphores();
    cleanup_shop();

}

void setup_shop(int seats_number){
    // TODO
}

void setup_semaphores(){

    // create semaphore
    shop_semaphore = semget(get_shop_key(), 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR);
    if(shop_semaphore == -1){
        perror("An error occurred while creating semaphore");
        exit(1);
    }

    // initialize semaphore
    union semun arg;
    arg.val = 0;
    if(semctl(shop_semaphore, 0, SETVAL, arg) == -1){
        perror("An error occurred while initializing semaphore");
        exit(1);
    }

}

void setup_sigint(){

    // sets up SIGINT handler
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

int main(int argc, char** args){

    // check arguments count
    if (argc < 2) {
        fprintf(stderr, "Missing arguments!\nUsage: <seats>\n\tseats - number of seats in waiting room\n");
        exit(1);
    }

    // parse arguments
    int seats_number = atoi(args[1]);

    // setup
    setup_atexit();
    setup_sigint();
    setup_semaphores();
    setup_shop(seats_number);

    // handle shop
    handle_shop();

    return 0;
}
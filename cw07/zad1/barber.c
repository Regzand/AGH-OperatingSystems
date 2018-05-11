#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "shop.h"

int shop_semaphore = -1;

int shmid = -1;
struct shop_data* shop;

void handle_shop(){

    while(1) {

        take_semaphore(shop_semaphore, SEMAPHORE_DATA);

        // if there are clients waiting in waiting room
        if (queue_length(shop) > 0) {

            // invite client
            int pid = queue_top(shop);
            printf("[%s] [BARBER] Invited client %d\n", get_time(), pid);
            send_signal(pid);
            give_semaphore(shop_semaphore, SEMAPHORE_DATA);

            // wait for client to sit on chair
            take_semaphore(shop_semaphore, SEMAPHORE_CHAIR);

        // waiting room is empty -> fall asleep
        } else {

            // falls asleep
            printf("[%s] [BARBER] Falls asleep\n", get_time());
            shop->barber_status = BARBER_STATUS_SLEEPING;
            give_semaphore(shop_semaphore, SEMAPHORE_DATA);

            // wait for client to wake him up
            take_semaphore(shop_semaphore, SEMAPHORE_CHAIR);
            printf("[%s] [BARBER] Wakes up\n", get_time());
        }

        printf("[%s] [BARBER] Starts cutting client %d\n", get_time(), shop->chair);
        printf("[%s] [BARBER] Ends cutting client %d\n", get_time(), shop->chair);

        // notify client
        give_semaphore(shop_semaphore, SEMAPHORE_DONE);
        
        // wait for client to leave
        take_semaphore(shop_semaphore, SEMAPHORE_CHAIR);

    }
}

void cleanup_shop(){

    // unmount shared memory
    if(shmdt(shop) == -1)
        perror("An error occurred while unmounting shared memory");

    // remove shared memory
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        perror("An error occurred while removing shared memory");

}

void cleanup_semaphores(){

    // remove semaphore
    if(semctl(shop_semaphore, -1, IPC_RMID) == -1)
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

    // create shared memory
    shmid = shmget(get_shop_key(), sizeof(struct shop_data), IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR);
    if(shmid == -1){
        perror("An error occurred while creating shared memory");
        exit(1);
    }

    // mount shared memory
    shop = (struct shop_data*) shmat(shmid, NULL, 0);
    if(shop == (struct shop_data*) -1){
        perror("An error occurred while mounting shared memory");
        exit(1);
    }

    // setup shop queue
    queue_init(shop, seats_number);

    // init barber
    shop->barber_pid = getpid();
    shop->barber_status = BARBER_STATUS_WORKING;

}

void setup_semaphores(){

    // create semaphore
    shop_semaphore = semget(get_shop_key(), 3, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR);
    if(shop_semaphore == -1){
        perror("An error occurred while creating semaphore");
        exit(1);
    }

    // initialize semaphore
    union semun arg;
    arg.val = 1;
    if(semctl(shop_semaphore, SEMAPHORE_DATA, SETVAL, arg) == -1){
        perror("An error occurred while initializing semaphore");
        exit(1);
    }

    arg.val = 0;
    if(semctl(shop_semaphore, SEMAPHORE_CHAIR, SETVAL, arg) == -1){
        perror("An error occurred while initializing semaphore");
        exit(1);
    }

    arg.val = 0;
    if(semctl(shop_semaphore, SEMAPHORE_DONE, SETVAL, arg) == -1){
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

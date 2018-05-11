#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "shop.h"

sem_t* semaphore_data;
sem_t* semaphore_chair;
sem_t* semaphore_done;

struct shop_data* shop;

void handle_shop(){

    while(1) {

        take_semaphore(semaphore_data);

        // if there are clients waiting in waiting room
        if (queue_length(shop) > 0) {

            // invite client
            int pid = queue_top(shop);
            printf("[%s] [BARBER] Invited client %d\n", get_time(), pid);
            send_signal(pid);
            give_semaphore(semaphore_data);

            // wait for client to sit on chair
            take_semaphore(semaphore_chair);

        // waiting room is empty -> fall asleep
        } else {

            // falls asleep
            printf("[%s] [BARBER] Falls asleep\n", get_time());
            shop->barber_status = BARBER_STATUS_SLEEPING;
            give_semaphore(semaphore_data);

            // wait for client to wake him up
            take_semaphore(semaphore_chair);
            printf("[%s] [BARBER] Wakes up\n", get_time());
        }

        printf("[%s] [BARBER] Starts cutting client %d\n", get_time(), shop->chair);
        printf("[%s] [BARBER] Ends cutting client %d\n", get_time(), shop->chair);

        // notify client
        give_semaphore(semaphore_done);
        
        // wait for client to leave
        take_semaphore(semaphore_chair);

    }
}

void cleanup_shop(){

    // unmount shared memory
    if(munmap(shop, sizeof(struct shop_data)) == -1)
        perror("An error occurred while unmounting shared memory");

    // remove shared memory
    if(shm_unlink(SHOP_NAME) == -1)
        perror("An error occurred while removing shared memory");

}

void cleanup_semaphores(){

    if(sem_close(semaphore_data) == -1)
        perror("An error occurred while closing data semaphore");

    if(sem_unlink(SEMAPHORE_DATA) == -1)
        perror("An error occurred while removing data semaphore");

    if(sem_close(semaphore_chair) == -1)
        perror("An error occurred while closing chair semaphore");

    if(sem_unlink(SEMAPHORE_CHAIR) == -1)
        perror("An error occurred while removing data semaphore");

    if(sem_close(semaphore_done) == -1)
        perror("An error occurred while closing done semaphore");

    if(sem_unlink(SEMAPHORE_DONE) == -1)
        perror("An error occurred while removing data semaphore");

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
    int fd = shm_open(SHOP_NAME, O_CREAT | O_EXCL | O_RDWR, S_IWUSR | S_IRUSR);
    if(fd == -1){
        perror("An error occurred while creating shared memory");
        exit(1);
    }

    // truncate memory
    if(ftruncate(fd, sizeof(struct shop_data)) == -1){
        perror("An error occurred while setting size of shared memory");
        exit(1);
    }

    // mount shared memory
    shop = (struct shop_data*) mmap(NULL, sizeof(struct shop_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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

    semaphore_data  = sem_open(SEMAPHORE_DATA,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
    semaphore_chair = sem_open(SEMAPHORE_CHAIR, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    semaphore_done  = sem_open(SEMAPHORE_DONE,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);

    if(semaphore_data == SEM_FAILED || semaphore_chair == SEM_FAILED || semaphore_done == SEM_FAILED){
        perror("An error occurred while creating semaphores");
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

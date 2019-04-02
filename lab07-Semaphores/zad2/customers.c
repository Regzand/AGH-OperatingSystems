#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "shop.h"

int notified = 0;

sem_t* semaphore_data;
sem_t* semaphore_chair;
sem_t* semaphore_done;

struct shop_data* shop;

int visit_shop(){

    take_semaphore(semaphore_data);

    // barber is sleeping -> wake him up
    if(shop->barber_status == BARBER_STATUS_SLEEPING){
        shop->barber_status = BARBER_STATUS_WORKING;
        printf("[%s] [%06d] Wakes up barber\n", get_time(), getpid());

    // barber is working go to waiting room
    }else{

        // if waiting room is full -> exit
        if(queue_length(shop) >= shop->queue_size){
            printf("[%s] [%06d] Leaves shop because waiting room is full\n", get_time(), getpid());
            give_semaphore(semaphore_data);
            return 0;
        }

        // temporary block signal
        sigset_t mask, old_mask;
        sigemptyset(&mask);
        sigaddset(&mask, SHOP_SIGNAL);
        sigprocmask(SIG_BLOCK, &mask, &old_mask);

        // if not sit in queue and wait for barber
        printf("[%s] [%06d] Sits down in waiting room\n", get_time(), getpid());
        queue_push(shop, getpid());

        notified = 0;

        give_semaphore(semaphore_data);

        while(notified == 0)
            sigsuspend(&old_mask);

        sigprocmask(SIG_SETMASK, &old_mask, NULL);

        take_semaphore(semaphore_data);
        queue_pop(shop);
    }

    // sit on chair and notify barber
    printf("[%s] [%06d] Sits down on the chair\n", get_time(), getpid());
    shop->chair = getpid();
    give_semaphore(semaphore_data);
    give_semaphore(semaphore_chair);


    // wait for barber to end and leave the shop and notify barber
    take_semaphore(semaphore_done);

    printf("[%s] [%06d] Leaves shop\n", get_time(), getpid());
    
    give_semaphore(semaphore_chair);
    
    return 1;
}

void cleanup_shop(){

    // unmount shared memory
    if(munmap(shop, sizeof(struct shop_data)) == -1)
        perror("An error occurred while unmounting shared memory");

}

void cleanup_semaphores(){

    if(sem_close(semaphore_data) == -1)
        perror("An error occurred while closing data semaphore");

    if(sem_close(semaphore_chair) == -1)
        perror("An error occurred while closing chair semaphore");

    if(sem_close(semaphore_done) == -1)
        perror("An error occurred while closing done semaphore");

}

void handle_exit(){

    // clean up
    cleanup_semaphores();
    cleanup_shop();

}

void handle_shop_signal(int signo){
    notified = 1;
}

void setup_shop(){

    // create shared memory
    int fd = shm_open(SHOP_NAME, O_RDWR, 0);
    if(fd == -1){
        perror("An error occurred while opening shared memory");
        exit(1);
    }

    // mount shared memory
    shop = (struct shop_data*) mmap(NULL, sizeof(struct shop_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shop == (struct shop_data*) -1){
        perror("An error occurred while mounting shared memory");
        exit(1);
    }

}

void setup_semaphores(){

    semaphore_data  = sem_open(SEMAPHORE_DATA,  0);
    semaphore_chair = sem_open(SEMAPHORE_CHAIR, 0);
    semaphore_done  = sem_open(SEMAPHORE_DONE,  0);

    if(semaphore_data == SEM_FAILED || semaphore_chair == SEM_FAILED || semaphore_done == SEM_FAILED){
        perror("An error occurred while opening semaphores");
        exit(1);
    }

}

void setup_shop_signal(){

//    // sets up process signal mask
//    sigset_t mask;
//    sigemptyset(&mask);
//    sigaddset(&mask, SHOP_SIGNAL);
//    if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1){
//        perror("An error occurred while setting up process signal mask");
//        exit(1);
//    }

    // sets up shop signal handler
    struct sigaction sa;
    sa.sa_handler = handle_shop_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SHOP_SIGNAL, &sa, NULL) == -1) {
        perror("An error occurred while setting up handler for shop signal");
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

void customer_main(int visits_number){

    // setup
    setup_atexit();
    setup_shop_signal();
    setup_semaphores();
    setup_shop();

    // visit shop
    for(int i = 0; i < visits_number; i += visit_shop());

    // this is THE END
    exit(0);
}

int main(int argc, char** args){

    // check arguments count
    if (argc < 3) {
        fprintf(stderr, "Missing arguments!\nUsage: <customers> <visits>\n\tcustomers - number of customers\n\tvisits - number of visits per customer\n");
        exit(1);
    }

    // parse arguments
    int customers_number = atoi(args[1]);
    int visits_number = atoi(args[2]);

    // create customers
    for(int i = 0; i < customers_number; i++){

        int pid = fork();

        if(pid < 0){
            perror("An error occurred while creating customer using fork()");
            exit(1);
        }

        if(pid == 0) {
            customer_main(visits_number);
            exit(666);
        }
    }

    // wait for customers to end
    while(wait(NULL) > 0);

    return 0;
}

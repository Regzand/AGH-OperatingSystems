#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#include "shop.c"

int shop_semaphore = -1;

void visit_shop(){
    // TODO
}

void cleanup_shop(){
    // TODO
}

void cleanup_semaphores(){
    // System V doesn't support closing semaphores
}

void handle_exit(){

    // clean up
    cleanup_semaphores();
    cleanup_shop();

}

void setup_shop(){
    // TODO
}

void setup_semaphores(){

    // get home path
    char* home = getenv("HOME");

    // get shop semaphore key
    int key = ftok(home, SHOP_SEMAPHORE_NUMBER);
    if(key == -1){
        perror("An error occurred while creating shop semaphore key");
        exit(1);
    }

    // get semaphore
    shop_semaphore = semget(key, 0, 0);
    if(shop_semaphore == -1){
        perror("An error occurred while getting semaphore");
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
    setup_semaphores();
    setup_shop();

    // visit shop
    for(int i = 0; i < visits_number; i++)
        visit_shop();

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
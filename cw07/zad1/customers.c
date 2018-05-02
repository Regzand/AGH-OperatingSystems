#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shop.c"

void visit_shop(){
    // TODO
}

void handle_atexit(){
    // TODO
}

void cleanup_shop(){
    // TODO
}

void cleanup_semaphores(){
    // TODO
}

void setup_shop(){
    // TODO
}

void setup_semaphores(){
    // TODO
}

void setup_atexit(){
    // TODO
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
            exit(2);
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

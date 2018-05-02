#include <stdio.h>
#include <stdlib.h>

#include "shop.c"

void handle_shop(){
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
    setup_semaphores();
    setup_shop();

    // handle shop
    handle_shop();

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

#include "shop.h"

int get_shop_key(){

    // get home path
    char* home = getenv("HOME");

    // get shop semaphore key
    int key = ftok(home, SHOP_KEY_NUMBER);
    if(key == -1){
        perror("An error occurred while creating shop key");
        exit(1);
    }

    return key;
}

void take_semaphore(int sem_id, int sem_num){
    // TODO
}

void give_semaphore(int sem_id, int sem_num){
    // TODO
}

void queue_init(struct shop_data* shop, int size){
    // TODO
}

void queue_push(struct shop_data* shop, int value){
    // TODO
}

int queue_pop(struct shop_data* shop){
    // TODO
}

int queue_length(struct shop_data* shop){
    // TODO
}

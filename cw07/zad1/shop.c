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
    shop -> queue_size = size;
    shop -> queue_head = 0;
    shop -> queue_tail = 0;
}

void queue_push(struct shop_data* shop, int value){
    if(queue_length(shop) >= shop -> queue_size) {
        fprintf(stderr, "Queue size exceeded!\n");
        exit(1);
    }

    shop -> queue[(shop -> queue_tail) % (shop -> queue_size)] = value;
    shop -> queue_tail += 1;
}

int queue_pop(struct shop_data* shop){
    if(queue_length(shop) == 0)
        return -1;

    int result = shop -> queue[(shop -> queue_head) % (shop -> queue_size)];
    shop -> queue_head += 1;

    return result;
}

int queue_length(struct shop_data* shop){
    return shop -> queue_tail - shop -> queue_head;
}

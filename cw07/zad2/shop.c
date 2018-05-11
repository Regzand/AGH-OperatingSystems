#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "shop.h"

void take_semaphore(sem_t* sem){

    if(sem_wait(sem) == -1){
        perror("An error occurred while taking semaphore");
        exit(1);
    }

}

void give_semaphore(sem_t* sem){

    if(sem_post(sem) == -1){
        perror("An error occurred while taking semaphore");
        exit(1);
    }

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

void send_signal(int pid){
    union sigval sv;
    if(sigqueue(pid, SHOP_SIGNAL, sv) == -1){
        perror("An error occurred while sending signal");
        exit(1);
    }

}

char* get_time(){

    // get time
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) == -1){
        perror("An error occurred while getting monotonic time");
        exit(1);
    }

    long N = time.tv_nsec / 1000;
    long S = time.tv_sec % 60;
    long M = time.tv_sec / 60 % 60;
    long H = time.tv_sec / 60 / 60;

    // time to string
    char* result = malloc(sizeof(char) * 16);
    sprintf(result, "%02ld:%02ld:%02ld.%06ld", H, M, S, N);

    return result;
}

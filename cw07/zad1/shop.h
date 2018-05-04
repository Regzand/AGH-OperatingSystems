#ifndef SHOP_H
#define SHOP_H

// number used in creating shop key
#define SHOP_KEY_NUMBER  13

// maximum seats in waiting room
#define SHOP_MAX_SEATS   20

// shop shared memory
typedef struct shop_data {
    int status;
    int queue_size;
    int queue_index;
    int queue[SHOP_MAX_SEATS];
} shop_data;

// needed for semaphore initialization
typedef union semun { int val; } semun;

// creates shop key (System V)
int get_shop_key();

#endif
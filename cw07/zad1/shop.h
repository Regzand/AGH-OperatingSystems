#ifndef SHOP_H
#define SHOP_H

// number used in creating shop key
#define SHOP_KEY_NUMBER         13

// maximum seats in waiting room
#define SHOP_MAX_SEATS          20

// shop statuses
#define SHOP_STATUS_SLEEPING    1
#define SHOP_STATUS_WORKING     2

// shop shared memory
typedef struct shop_data {
    int status;
    int queue_size;
    int queue_head;
    int queue_tail;
    int queue[SHOP_MAX_SEATS];
} shop_data;

// needed for semaphore initialization
typedef union semun { int val; } semun;

// creates shop key (System V)
int get_shop_key();

// handling shop semaphores
void take_semaphore(int sem_id, int sem_num);
void give_semaphore(int sem_id, int sem_num);

// queue handling functions
void queue_init(struct shop_data* shop, int size);
void queue_push(struct shop_data* shop, int value);
int queue_length(struct shop_data* shop);
int queue_pop(struct shop_data* shop);

// signals
void wait_for_signal();
void send_signal(int pid);

// timing function
char* get_time();

#endif
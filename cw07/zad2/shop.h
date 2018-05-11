#ifndef SHOP_H
#define SHOP_H

// shop memory name
#define SHOP_NAME               "/barber-shop"

// maximum seats in waiting room
#define SHOP_MAX_SEATS          20

// shop statuses
#define BARBER_STATUS_SLEEPING  1
#define BARBER_STATUS_WORKING   2

// signal used for waking up barber and inviting clients
#define SHOP_SIGNAL             SIGRTMIN

// semaphores
#define SEMAPHORE_DATA          "/sem-data"
#define SEMAPHORE_CHAIR         "/sem-chair"
#define SEMAPHORE_DONE          "/sem-done"

// shop shared memory
typedef struct shop_data {
    int chair;

    int barber_status;
    int barber_pid;

    int queue_size;
    int queue_head;
    int queue_tail;
    int queue[SHOP_MAX_SEATS];
} shop_data;

// handling shop semaphores
void take_semaphore(sem_t* sem);
void give_semaphore(sem_t* sem);

// queue handling functions
void queue_init(struct shop_data* shop, int size);
void queue_push(struct shop_data* shop, int value);
int queue_length(struct shop_data* shop);
int queue_pop(struct shop_data* shop);
int queue_top(struct shop_data* shop);

// signals
void send_signal(int pid);

// timing function
char* get_time();

#endif

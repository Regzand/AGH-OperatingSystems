#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// colors
#define RED         "\x1B[31m"
#define GREEN       "\x1B[32m"
#define YELLOW      "\x1B[33m"
#define BLUE        "\x1B[34m"
#define MAGENTA     "\x1B[35m"
#define CYAN        "\x1B[36m"
#define WHITE       "\x1B[37m"
#define RESET       "\x1B[0m"

// output levels
#define OUTPUT_SIMPLE 0
#define OUTPUT_VERBOSE 1

// mutexes
pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_queue = PTHREAD_COND_INITIALIZER;

// queue
char** queue;
int    queue_head;
int    queue_tail;

// settings
int    producers_number;
int    consumers_number;
int    queue_size;
int    search_length;
char   search_operator;
int    output_mode;
int    limit;
FILE*  input;

int finished = 0;

int check_condition(int length){
    if(search_operator == '<')
        return length < search_length;
    if(search_operator == '>')
        return length > search_length;
    if(search_operator == '=')
        return length == search_length;
    return 0;
}

// ==================================================================
// = QUEUE OPERATIONS
// ==================================================================

void queue_init(){
    queue = malloc(sizeof(char*) * queue_size);
    queue_head = 0;
    queue_tail = 0;
}

int queue_length(){
    return queue_tail - queue_head;
}

void queue_push(char* value){
    if(queue_length() >= queue_size) {
        fprintf(stderr, "Queue size exceeded!\n");
        exit(1);
    }

    queue[(queue_tail) % (queue_size)] = value;
    queue_tail += 1;
}

char* queue_pop(){
    if(queue_length() == 0)
        return NULL;

    char* result = queue[(queue_head) % (queue_size)];
    queue_head += 1;

    return result;
}

char* queue_top(){
    if(queue_length() == 0)
        return NULL;

    return queue[(queue_head) % (queue_size)];
}


// ==================================================================
// = PRODUCERS
// ==================================================================

/**
 * @return 0 if successfully produced; -1 if there is nothing left to produce
 */
int produce(){

    // locks queue
    pthread_mutex_lock(&mutex_queue);

    // wait for empty slot in queue
    while(queue_length() >= queue_size)
        pthread_cond_wait(&cond_queue, &mutex_queue);

    // read line
    char* line = NULL;
    size_t len = 0;
    int chars = -1;
    if((chars = getline(&line, &len, input)) == -1){

        // logging
        if(output_mode == OUTPUT_VERBOSE)
            printf("[Producer %ld] Encountered EOF - nothing to produce\n", pthread_self());

        // notify consumers
        finished = 1;
        pthread_cond_broadcast(&cond_queue);

        // nothing left in input
        pthread_mutex_unlock(&mutex_queue);
        return -1;
    }

    // get rid of new line char
    line[chars-1] = '\0';

    // add line to queue
    queue_push(line);

    // logging
    if(output_mode == OUTPUT_VERBOSE)
        printf("[Producer %ld] Produced: %s%s%s\n", pthread_self(), GREEN, line, RESET);

    // notify about changes in queue
    pthread_cond_broadcast(&cond_queue);

    // unlocks queue
    pthread_mutex_unlock(&mutex_queue);

    return 0;
}

void *main_producer(){

    // logging
    if(output_mode == OUTPUT_VERBOSE)
        printf("[Producer %ld] Created\n", pthread_self());

    // producing
    while(1)
        if(produce() != 0)
            break;

    // logging
    if(output_mode == OUTPUT_VERBOSE)
        printf("[Producer %ld] Finished\n", pthread_self());

    return 0;
}


// ==================================================================
// = CONSUMERS
// ==================================================================

/**
 * @return 0 if successfully consumed; -1 if there is nothing left to consume
 */
int consume(){

    // locks queue
    pthread_mutex_lock(&mutex_queue);

    // wait for anything to appear in queue or for notification about end
    while(queue_length() == 0 && !finished)
        pthread_cond_wait(&cond_queue, &mutex_queue);

    // if queue is empty it THE END
    if(queue_length() == 0){
        // logging
        if(output_mode == OUTPUT_VERBOSE)
            printf("[Consumer %ld] Production ended - nothing to consume\n", pthread_self());

        pthread_mutex_unlock(&mutex_queue);
        return -1;
    }

    // get line from queue
    char* line = queue_pop();

    // logging
    if(output_mode == OUTPUT_VERBOSE)
        printf("[Consumer %ld] Consumed: %s%s %s%s%s\n", pthread_self(), MAGENTA, line, YELLOW, (check_condition(strlen(line)) ? "[OK]" : ""), RESET);

    // output
    if(output_mode == OUTPUT_SIMPLE && check_condition(strlen(line)))
        printf("%s\n", line);

    // remove line from memory
    free(line);

    // notify about changes in queue
    pthread_cond_broadcast(&cond_queue);

    // unlocks queue
    pthread_mutex_unlock(&mutex_queue);

    return 0;
}

void *main_consumer(){

    // logging
    if(output_mode == OUTPUT_VERBOSE)
        printf("[Consumer %ld] Created\n", pthread_self());

    // consuming
    while(1)
        if(consume() != 0)
            break;

    // logging
    if(output_mode == OUTPUT_VERBOSE)
        printf("[Consumer %ld] Finished\n", pthread_self());

    return 0;
}


// ==================================================================
// = MAIN PROGRAM
// ==================================================================

void load_settings(const char* file){

    // open settings file
    FILE* fp = fopen(file, "r");
    if (fp == NULL){
        perror("An error occurred while opening settings file");
        exit(1);
    }

    // load settings
    char input_name[64];
    fscanf(fp, "%d %d %d %s %d %c %d %d",
           &producers_number,
           &consumers_number,
           &queue_size,
           input_name,
           &search_length,
           &search_operator,
           &output_mode,
           &limit
    );

    // open input file
    input = fopen(input_name, "r");

    // verify input
    if (input == NULL){
        perror("An error occurred while opening input file");
        exit(1);
    }
    if (producers_number < 1){
        fprintf(stderr, "Producers number must ba a positive integer\n");
        exit(1);
    }
    if (consumers_number < 1){
        fprintf(stderr, "Consumers number must ba a positive integer\n");
        exit(1);
    }
    if (queue_size < 1){
        fprintf(stderr, "Buffer size must ba a positive integer\n");
        exit(1);
    }
    if (search_length < 1){
        fprintf(stderr, "Search length size must ba a positive integer\n");
        exit(1);
    }
    if (!(search_operator == '<' || search_operator == '=' || search_operator == '>')){
        fprintf(stderr, "Search operator must be one of following: '<' '=' '>'\n");
        exit(1);
    }
    if (output_mode < 0 || output_mode > 1){
        fprintf(stderr, "Output mode must be one of following: 0 - simple, 1 - verbose\n");
        exit(1);
    }
    if (limit < 0){
        fprintf(stderr, "Limit must ba a positive integer or zero (no limit)\n");
        exit(1);
    }

    // close settings file
    fclose(fp);

}

int main(int argc, char **args) {

    // check arguments count
    if (argc < 2) {
        fprintf(stderr, "Missing arguments!\nUsage: <settings file>\nSettings file format: <producers> <consumers> <buffer size> <input file> <search length> <search operator> <output_mode> <limit>\n");
        exit(1);
    }

    // load settings
    load_settings(args[1]);

    // create queue
    queue_init();

    // array for threads ids
    pthread_t* tid = malloc(sizeof(pthread_t) * (producers_number + consumers_number));

    // create producers threads
    for(int i = 0; i < producers_number; i++) {
        if (pthread_create(&tid[i], NULL, main_producer, NULL) != 0) {
            perror("An error occurred while creating producer thread");
            exit(1);
        }
    }


    // create consumers threads
    for(int i = producers_number; i < (producers_number + consumers_number); i++) {
        if (pthread_create(&tid[i], NULL, main_consumer, NULL) != 0) {
            perror("An error occurred while creating consumer thread");
            exit(1);
        }
    }

    // if there was an limit sleep and then exit
    if (limit > 0){
        sleep(limit);

        // logging
        if(output_mode == OUTPUT_VERBOSE)
            printf("Time limit reaches - exiting\n");

    // if there was no limit wait for all threads to finish
    } else {
        for(int i = 0; i < (producers_number + consumers_number); i++){
            if (pthread_join(tid[i], NULL) != 0) {
                perror("An error occurred while waiting for thread to finish");
                exit(1);
            }
        }

        // logging
        if(output_mode == OUTPUT_VERBOSE)
            printf("All threads finished - exiting\n");
    }

    return 0;
}
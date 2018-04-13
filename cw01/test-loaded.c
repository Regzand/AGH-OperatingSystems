#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dlfcn.h>

#include "dynamic_array.h"
#include "static_array.h"
#include "timing.h"


void (*dll_startTimer)(timer*);
void (*dll_stopTimer)(timer*);
void (*dll_displayTime)(timer*, char*);

void (*dll_init_blockarray_static)(blockarray_static*, int);
void (*dll_add_block_static)(blockarray_static*, int, char*, int);
void (*dll_free_block_static)(blockarray_static*, int);
int  (*dll_find_block_static)(blockarray_static*, int);

void (*dll_init_blockarray_dynamic)(blockarray_dynamic*, int);
void (*dll_free_blockarray_dynamic)(blockarray_dynamic*);
void (*dll_add_block_dynamic)(blockarray_dynamic*, int, char*, int);
void (*dll_free_block_dynamic)(blockarray_dynamic*, int);
int  (*dll_find_block_dynamic)(blockarray_dynamic*, int);

char* random_string(int length){
    char* res = calloc(length, sizeof(char));
    while(length)
        res[--length] = (char)(rand()%95 + 33);
    return res;
}

void main_static(int argc, char** args){

    // sizes
    int array_size = atoi(args[2]);
    int block_size = atoi(args[3]);

    // info
    printf("Testing statically allocated array of size %d (block size: %d)...\n\n", array_size, block_size);

    // timing
    struct timer T;

    //=============================================================
    // INITIALIZATION
    //=============================================================

    (*dll_startTimer)(&T);

    blockarray_static array;
    dll_init_blockarray_static(&array, array_size);

    dll_stopTimer(&T);
    dll_displayTime(&T, "static init");

    //=============================================================
    // COMMANDS EXECUTION
    //=============================================================

    int i = 4;
    while(i < argc){

        //=============================================================
        // SEARCH ELEMENT
        //=============================================================
        if(strcmp(args[i], "search_element") == 0){
            // parameters
            int index = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: search_element %d\n", index);

            dll_startTimer(&T);

            dll_find_block_static(&array, index);

            dll_stopTimer(&T);
            dll_displayTime(&T, "static search");

        //=============================================================
        // REMOVE
        //=============================================================
        }else if(strcmp(args[i], "remove") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove %d\n", number);

            dll_startTimer(&T);

            while(number)
                dll_free_block_static(&array, --number);

            dll_stopTimer(&T);
            dll_displayTime(&T, "static remove");

        //=============================================================
        // ADD
        //=============================================================
        }else if(strcmp(args[i], "add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: add %d\n", number);

            dll_startTimer(&T);

            while(number)
                dll_add_block_static(&array, --number, random_string(block_size), block_size);

            dll_stopTimer(&T);
            dll_displayTime(&T, "static add");

        //=============================================================
        // REMOVE AND ADD
        //=============================================================
        }else if(strcmp(args[i], "remove_and_add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove_and_add %d\n", number);

            dll_startTimer(&T);

            while(number){
                number--;
                dll_free_block_static(&array, number);
                dll_add_block_static(&array, number, random_string(block_size), block_size);
            }

            dll_stopTimer(&T);
            dll_displayTime(&T, "static init");

        }else{
            fprintf(stderr, "Unknown command\n");
            exit(3);
        }

    }
}

void main_dynamic(int argc, char** args){

    // sizes
    int array_size = atoi(args[2]);
    int block_size = atoi(args[3]);

    // info
    printf("Testing dynamically allocated array of size %d (block size: %d)...\n\n", array_size, block_size);

    // timing
    struct timer T;

    //=============================================================
    // INITIALIZATION
    //=============================================================

    dll_startTimer(&T);

    blockarray_dynamic array;
    dll_init_blockarray_dynamic(&array, array_size);

    dll_stopTimer(&T);
    dll_displayTime(&T, "dynamic init");

    //=============================================================
    // COMMANDS EXECUTION
    //=============================================================

    int i = 4;
    while(i < argc){

        //=============================================================
        // SEARCH ELEMENT
        //=============================================================
        if(strcmp(args[i], "search_element") == 0){
            // parameters
            int index = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: search_element %d\n", index);

            dll_startTimer(&T);

            dll_find_block_dynamic(&array, index);

            dll_stopTimer(&T);
            dll_displayTime(&T, "dynamic search");

            //=============================================================
            // REMOVE
            //=============================================================
        }else if(strcmp(args[i], "remove") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove %d\n", number);

            dll_startTimer(&T);

            while(number)
                dll_free_block_dynamic(&array, --number);

            dll_stopTimer(&T);
            dll_displayTime(&T, "dynamic remove");

            //=============================================================
            // ADD
            //=============================================================
        }else if(strcmp(args[i], "add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: add %d\n", number);

            dll_startTimer(&T);

            while(number)
                dll_add_block_dynamic(&array, --number, random_string(block_size), block_size);

            dll_stopTimer(&T);
            dll_displayTime(&T, "dynamic add");

            //=============================================================
            // REMOVE AND ADD
            //=============================================================
        }else if(strcmp(args[i], "remove_and_add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove_and_add %d\n", number);

            dll_startTimer(&T);

            while(number){
                number--;
                dll_free_block_dynamic(&array, number);
                dll_add_block_dynamic(&array, number, random_string(block_size), block_size);
            }

            dll_stopTimer(&T);
            dll_displayTime(&T, "dynamic init");

        }else{
            fprintf(stderr, "Unknown command\n");
            exit(3);
        }
    }
}

int main(int argc, char** args){
    srand(time(NULL));

    // ======== DLL ========

    void* handle = dlopen("libarrays.so", RTLD_LAZY);
    if(!handle){
        fprintf(stderr, "An error occurred while loading arrays library");
        exit(10);
    }

    dll_startTimer              = dlsym(handle, "startTimer");
    if(dlerror()) printf("eeee\n");
    dll_stopTimer               = dlsym(handle, "stopTimer");
    dll_displayTime             = dlsym(handle, "displayTime");

    dll_init_blockarray_static  = dlsym(handle, "init_blockarray_static");
    dll_add_block_static        = dlsym(handle, "add_block_static");
    dll_free_block_static       = dlsym(handle, "free_block_static");
    dll_find_block_static       = dlsym(handle, "find_block_static");

    dll_init_blockarray_dynamic = dlsym(handle, "init_blockarray_dynamic");
    dll_free_blockarray_dynamic = dlsym(handle, "free_blockarray_dynamic");
    dll_add_block_dynamic       = dlsym(handle, "add_block_dynamic");
    dll_free_block_dynamic      = dlsym(handle, "free_block_dynamic");
    dll_find_block_dynamic      = dlsym(handle, "find_block_dynamic");

    // ======================

    // check arguments count
    if(argc < 4){
        fprintf(stderr, "Missing arguments!\nExpected: <static|dynamic> <array size> <block size>\n");
        exit(1);
    }

    // check allocation type
    if(strcmp(args[1], "dynamic") == 0) {
        main_dynamic(argc, args);

    }else if(strcmp(args[1], "static") == 0) {
        main_static(argc, args);

    }else{
        fprintf(stderr, "First argument has to be 'static' or 'dynamic'\n");
        exit(2);
    }

    dlclose(handle);
    return 0;
}
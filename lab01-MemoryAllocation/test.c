#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "dynamic_array.h"
#include "static_array.h"
#include "timing.h"

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

    startTimer(&T);

    blockarray_static array;
    init_blockarray_static(&array, array_size);

    stopTimer(&T);
    displayTime(&T, "static init");

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

            startTimer(&T);

            find_block_static(&array, index);

            stopTimer(&T);
            displayTime(&T, "static search");

        //=============================================================
        // REMOVE
        //=============================================================
        }else if(strcmp(args[i], "remove") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove %d\n", number);

            startTimer(&T);

            while(number)
                free_block_static(&array, --number);

            stopTimer(&T);
            displayTime(&T, "static remove");

        //=============================================================
        // ADD
        //=============================================================
        }else if(strcmp(args[i], "add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: add %d\n", number);

            startTimer(&T);

            while(number)
                add_block_static(&array, --number, random_string(block_size), block_size);

            stopTimer(&T);
            displayTime(&T, "static add");

        //=============================================================
        // REMOVE AND ADD
        //=============================================================
        }else if(strcmp(args[i], "remove_and_add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove_and_add %d\n", number);

            startTimer(&T);

            while(number){
                number--;
                free_block_static(&array, number);
                add_block_static(&array, number, random_string(block_size), block_size);
            }

            stopTimer(&T);
            displayTime(&T, "static init");

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

    startTimer(&T);

    blockarray_dynamic array;
    init_blockarray_dynamic(&array, array_size);

    stopTimer(&T);
    displayTime(&T, "dynamic init");

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

            startTimer(&T);

            find_block_dynamic(&array, index);

            stopTimer(&T);
            displayTime(&T, "dynamic search");

            //=============================================================
            // REMOVE
            //=============================================================
        }else if(strcmp(args[i], "remove") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove %d\n", number);

            startTimer(&T);

            while(number)
                free_block_dynamic(&array, --number);

            stopTimer(&T);
            displayTime(&T, "dynamic remove");

            //=============================================================
            // ADD
            //=============================================================
        }else if(strcmp(args[i], "add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: add %d\n", number);

            startTimer(&T);

            while(number)
                add_block_dynamic(&array, --number, random_string(block_size), block_size);

            stopTimer(&T);
            displayTime(&T, "dynamic add");

            //=============================================================
            // REMOVE AND ADD
            //=============================================================
        }else if(strcmp(args[i], "remove_and_add") == 0){
            // parameters
            int number = atoi(args[i+1]);
            i += 2;

            // info
            printf("Command: remove_and_add %d\n", number);

            startTimer(&T);

            while(number){
                number--;
                free_block_dynamic(&array, number);
                add_block_dynamic(&array, number, random_string(block_size), block_size);
            }

            stopTimer(&T);
            displayTime(&T, "dynamic init");

        }else{
            fprintf(stderr, "Unknown command\n");
            exit(3);
        }
    }
}

int main(int argc, char** args){
    srand(time(NULL));

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

}
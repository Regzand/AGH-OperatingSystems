#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "timing.h"

void generate_record(char* buffer, int size){
    while(size)
        buffer[--size] = (char)(rand()%95 + 33);
}

void sort_sys(char* file_name, int records, int size){

    // timing
    struct timer T;
    startTimer(&T);

    // open files
    int file = open(file_name, O_RDWR);
    if(file < 0){
        fprintf(stderr, "An error occurred while opening file!\n");
        exit(12);
    }

    // buffers
    char* record = malloc(size * sizeof(char));
    char* swap   = malloc(size * sizeof(char));

    // for every record (except first one)
    for(int i = 1; i < records; i++){

        // setting file pointer to the beginning of record
        if(lseek(file, i * size * sizeof(char), SEEK_SET) < 0){
            fprintf(stderr, "An error occurred while setting file pointer!\n");
            exit(14);
        }

        // loading record
        if(read(file, record, size * sizeof(char)) != size * sizeof(char)){
            fprintf(stderr, "An error occurred while reading from file!\n");
            exit(13);
        }

        // index at which this record will be inserted
        int index = 0;

        // search for place for this record
        for(int j = i; j > 0; j--){

            // setting file pointer to record
            if(lseek(file, (j-1) * size * sizeof(char), SEEK_SET) < 0){
                fprintf(stderr, "An error occurred while setting file pointer!\n");
                exit(14);
            }

            // loading record to swap
            if(read(file, swap, size * sizeof(char)) != size * sizeof(char)){
                fprintf(stderr, "An error occurred while reading from file!\n");
                exit(13);
            }

            // if this is not the right place
            if((unsigned char) swap[0] > (unsigned char) record[0]){

                if(lseek(file, j * size * sizeof(char), SEEK_SET) < 0){
                    fprintf(stderr, "An error occurred while setting file pointer!\n");
                    exit(14);
                }

                if(write(file, swap, size * sizeof(char)) != size * sizeof(char)){
                    fprintf(stderr, "An error occurred while writing to file!\n");
                    exit(14);
                }

                // if this is the right place
            }else{
                index = j;
                break;
            }

        }

        // insert record in the right place
        if(lseek(file, index * size * sizeof(char), SEEK_SET) < 0){
            fprintf(stderr, "An error occurred while setting file pointer!\n");
            exit(14);
        }

        if(write(file, record, size * sizeof(char)) != size * sizeof(char)){
            fprintf(stderr, "An error occurred while writing to file!\n");
            exit(14);
        }

    }

    // close files
    if(close(file) < 0){
        fprintf(stderr, "An error occurred while closing file!\n");
        exit(11);
    }

    // timing
    stopTimer(&T);
    displayTime(&T, "sort sys");

}

void sort_lib(char* file_name, int records, int size){

    // timing
    struct timer T;
    startTimer(&T);

    // open files
    FILE* file = fopen(file_name, "r+");
    if(file == NULL){
        fprintf(stderr, "An error occurred while opening file!\n");
        exit(12);
    }

    // buffers
    char* record = malloc(size * sizeof(char));
    char* swap   = malloc(size * sizeof(char));

    // for every record (except first one)
    for(int i = 1; i < records; i++){

        // setting file pointer to the beginning of record
        if(fseek(file, i * size * sizeof(char), SEEK_SET) != 0){
            fprintf(stderr, "An error occurred while setting file pointer!\n");
            exit(14);
        }

        // loading record
        if(fread(record, sizeof(char), size, file) != size){
            fprintf(stderr, "An error occurred while reading from file!\n");
            exit(13);
        }

        // index at which this record will be inserted
        int index = 0;

        // search for place for this record
        for(int j = i; j > 0; j--){

            // setting file pointer to record
            if(fseek(file, (j-1) * size * sizeof(char), SEEK_SET) != 0){
                fprintf(stderr, "An error occurred while setting file pointer!\n");
                exit(14);
            }

            // loading record to swap
            if(fread(swap, sizeof(char), size, file) != size){
                fprintf(stderr, "An error occurred while reading from file!\n");
                exit(13);
            }

            // if this is not the right place
            if((unsigned char) swap[0] > (unsigned char) record[0]){

                if(fseek(file, j * size * sizeof(char), SEEK_SET) != 0){
                    fprintf(stderr, "An error occurred while setting file pointer!\n");
                    exit(14);
                }

                if(fwrite(swap, sizeof(char), size, file) != size){
                    fprintf(stderr, "An error occurred while writing to file!\n");
                    exit(14);
                }

            // if this is the right place
            }else{
                index = j;
                break;
            }

        }

        // insert record in the right place
        if(fseek(file, index * size * sizeof(char), SEEK_SET) != 0){
            fprintf(stderr, "An error occurred while setting file pointer!\n");
            exit(14);
        }

        if(fwrite(record, sizeof(char), size, file) != size){
            fprintf(stderr, "An error occurred while writing to file!\n");
            exit(14);
        }

    }

    // close files
    if(fclose(file) != 0){
        fprintf(stderr, "An error occurred while closing file!\n");
        exit(11);
    }

    // timing
    stopTimer(&T);
    displayTime(&T, "sort lib");

}

void copy_sys(char* file1_name, char* file2_name, int records, int size){

    // timing
    struct timer T;
    startTimer(&T);

    // open files (and creating output file if needed)
    int input  = open(file1_name, O_RDONLY);
    int output = open(file2_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // rw-r--r--
    if(input < 0 || output < 0){
        fprintf(stderr, "An error occurred while opening files!\n");
        exit(12);
    }

    // buffer
    char* buffer = malloc(size * sizeof(char));

    // reading to buffer and writing from buffer to file
    for(int i = 0; i < records; i++){

        if(read(input, buffer, size * sizeof(char)) != size){
            fprintf(stderr, "An error occurred while reading from file!\n");
            exit(13);
        }

        if(write(output, buffer, size * sizeof(char)) != size){
            fprintf(stderr, "An error occurred while writing to file!\n");
            exit(14);
        }

    }

    // close files
    if(close(input) < 0 || close(output) < 0){
        fprintf(stderr, "An error occurred while closing files!\n");
        exit(11);
    }

    // timing
    stopTimer(&T);
    displayTime(&T, "copy sys");

}

void copy_lib(char* file1_name, char* file2_name, int records, int size){

    // timing
    struct timer T;
    startTimer(&T);

    // open files
    FILE* input  = fopen(file1_name, "r");
    FILE* output = fopen(file2_name, "w");
    if(input == NULL || output == NULL){
        fprintf(stderr, "An error occurred while opening files!\n");
        exit(12);
    }

    // buffer
    char* buffer = malloc(size * sizeof(char));

    // reading to buffer and writing from buffer to file
    for(int i = 0; i < records; i++){

        if(fread(buffer, sizeof(char), size, input) != size){
            fprintf(stderr, "An error occurred while reading from file!\n");
            exit(13);
        }

        if(fwrite(buffer, sizeof(char), size, output) != size){
            fprintf(stderr, "An error occurred while writing to file!\n");
            exit(14);
        }

    }

    // close files
    if(fclose(input) != 0 || fclose(output) != 0){
        fprintf(stderr, "An error occurred while closing files!\n");
        exit(11);
    }

    // timing
    stopTimer(&T);
    displayTime(&T, "copy lib");

}

void generate(char* file_name, int records, int size){

    // timing
    struct timer T;
    startTimer(&T);

    // open file
    FILE* file = fopen(file_name, "w");
    if(file == NULL){
        fprintf(stderr, "An error occurred while opening file!\n");
        exit(12);
    }

    // buffer
    char* buffer = malloc(size * sizeof(char));

    // generating and saving to file
    for(int i = 0; i < records; i++){
        generate_record(buffer, size);

        if(fwrite(buffer, sizeof(char), size, file) != size){
            fprintf(stderr, "An error occurred while writing to file!\n");
            exit(13);
        }
    }

    // close file
    if(fclose(file) != 0){
        fprintf(stderr, "An error occurred while closing file!\n");
        exit(11);
    }

    // timing
    stopTimer(&T);
    displayTime(&T, "generate");
}

void sort(char* file_name, int records, int size, char* method){

    // choose method
    if(strcmp(method, "sys") == 0){
        sort_sys(file_name, records, size);
    }else if(strcmp(method, "lib") == 0){
        sort_lib(file_name, records, size);
    }else{
        fprintf(stderr, "Wrong argument!\nMethod can only be 'sys' or 'lib'\n");
        exit(4);
    }

}

void copy(char* file1_name, char* file2_name, int records, int size, char* method){

    // choose method
    if(strcmp(method, "sys") == 0){
        copy_sys(file1_name, file2_name, records, size);
    }else if(strcmp(method, "lib") == 0){
        copy_lib(file1_name, file2_name, records, size);
    }else{
        fprintf(stderr, "Wrong argument!\nMethod can only be 'sys' or 'lib'\n");
        exit(4);
    }

}

int main(int argc, char** args){
    srand(time(NULL));

    // check arguments count
    if(argc < 4){
        fprintf(stderr, "Missing arguments!\nUsage: <records> <size> <command>\nCommands:\n\tgenerate <file>\n\tsort <file> <sys|lib>\n\tcopy <file from> <file to> <sys|lib>\n");
        exit(1);
    }

    // read records and size
    int records = atoi(args[1]);
    int size = atoi(args[2]);

    // execute command
    if(strcmp(args[3], "generate") == 0){

        // verify arguments
        if(argc < 5){
            fprintf(stderr, "Missing arguments!\nUsage: <records> <size> generate <file>\n");
            exit(3);
        }

        // display info
        printf("Generating '%s' file with %d random records of size %d...\n", args[4], records, size);

        // execute
        generate(args[4], records, size);

    }else if(strcmp(args[3], "sort") == 0){

        // verify arguments
        if(argc < 6){
            fprintf(stderr, "Missing arguments!\nUsage: <records> <size> sort <file> <sys|lib>\n");
            exit(3);
        }

        // display info
        printf("Sorting '%s' file with %d records of size %d, using %s...\n", args[4], records, size, args[5]);

        // execute
        sort(args[4], records, size, args[5]);

    }else if(strcmp(args[3], "copy") == 0){

        // verify arguments
        if(argc < 7){
            fprintf(stderr, "Missing arguments!\nUsage: <records> <size> copy <file from> <file to> <sys|lib>\n");
            exit(3);
        }

        // display info
        printf("Copying '%s' file with %d records of size %d to '%s', using %s...\n", args[4], records, size, args[5], args[6]);

        // execute
        copy(args[4], args[5], records, size, args[6]);

    }else {
        fprintf(stderr, "Unknown command!\n");
        exit(2);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char** args) {

    // check arguments count
    if (argc < 2) {
        fprintf(stderr, "Missing arguments!\nUsage: <path>\n");
        exit(1);
    }

    // create fifo
    if(mkfifo(args[1], S_IWUSR | S_IRUSR) == -1){
        perror("An error occurred while creating fifo");
        exit(2);
    }

    // open fifo
    FILE* fifo = fopen(args[1], "r");
    if(fifo == NULL){
        perror("An error occurred while opening fifo");
        exit(3);
    }

    // read line by line and print to stdout
    char* line = NULL;
    size_t line_size = 0;
    while(getline(&line, &line_size, fifo) > 0)
        printf("%s", line);

    // close fifo
    fclose(fifo);

    // remove fifo
    if(remove(args[1]) == -1){
        perror("An error occurred while removing fifo");
        exit(4);
    }

    return 0;
}
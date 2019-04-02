#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** args) {
    srand(time(NULL));

    // check arguments count
    if (argc < 3) {
        fprintf(stderr, "Missing arguments!\nUsage: <path> <N>\n");
        exit(1);
    }

    // parse input
    int N = atoi(args[2]);

    // print info
    fprintf(stdout, "Slave created - pid: %d\n", getpid());

    // open fifo
    int fifo = open(args[1], O_WRONLY);
    if(fifo < 0){
        perror("An error occurred while opening fifo");
        exit(3);
    }

    // repeat N times
    for(int i = 0; i < N; i++){

        // open date using popen
        FILE* fp_date = popen("date", "r");
        if(fp_date == NULL){
            perror("An error occurred while running date using popen");
            exit(4);
        }

        // read from date
        char* date = NULL;
        size_t date_size = 0;
        if(getline(&date, &date_size, fp_date) < 0){
            perror("An error occurred while reading date");
            exit(5);
        }

        // close date
        if(pclose(fp_date) == -1){
            perror("An error occurred while closing date");
            exit(6);
        }

        // combine pid and date
        char* out = malloc(sizeof(char) * 100);
        sprintf(out, "PID: %d - %s", getpid(), date);

        // write to fifo
        if(write(fifo, out, strlen(out) * sizeof(char)) < 0){
            perror("An error occurred while writing to fifo");
            exit(7);
        }

        // sleep
        sleep(rand() % (5-2+1) + 2);

    }

    // close fifo
    close(fifo);

    return 0;
}
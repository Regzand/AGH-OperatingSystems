#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>

#include "timing.h"

#define MAX_ARGS 20

int main(int argc, char** args) {

    // check arguments count
    if (argc < 4) {
        fprintf(stderr, "Missing arguments!\nUsage: <path> <mem limit> <cpu limit>\n");
        exit(1);
    }

    // prepares memory limits (shit by 20 to get value in MB)
    struct rlimit as_limit;
    as_limit.rlim_cur = (((long long int) atoi(args[2])) << 20);
    as_limit.rlim_max = as_limit.rlim_cur;

    // prepares cpu limits
    struct rlimit cpu_limit;
    cpu_limit.rlim_cur = atoi(args[3]);
    cpu_limit.rlim_max = cpu_limit.rlim_cur;

    // create timer
    struct timer T;
    T.RUSAGE_FLAG = RUSAGE_CHILDREN;

    // opens file
    FILE* input = fopen(args[1], "r");
    if(input == NULL){
        fprintf(stderr, "An error occurred while opening file with commands!\n");
        exit(2);
    }

    // reads file line by line
    char* line;
    size_t line_size;
    while(getline(&line, &line_size, input) != -1){

        for(int i = 0; line[i] != '\0'; i++)
            if(isspace(line[i]))
                line[i] = ' ';;

        // gets first word in a line -> file to be executed
        char* cmd = strtok(line, " ");

        // check if it is not an empty line
        if(cmd == NULL)
            continue;

        // parse rest of line -> command arguments
        char* cmd_args[MAX_ARGS + 1];
        cmd_args[0] = cmd;
        int cmd_argc = 1;

        while(cmd_argc <= MAX_ARGS){
            cmd_args[cmd_argc] = strtok(NULL, " ");

            if(cmd_args[cmd_argc] == NULL)
                break;

            cmd_argc++;
        }

        // displays info
        printf("< Executing: ");
        for(int i = 0; i < cmd_argc; i++)
            printf("%s ", cmd_args[i]);
        printf("\n");

        // creates child process
        pid_t process_id = fork();
        if(process_id < 0){
            fprintf(stderr, "En error occurred while creating process with fork()\n");
            exit(100);
        }

        // start timer
        start_timer(&T);

        // execute cmd in child precess
        if(process_id == 0){

            // sets cpu and memory limits for child process
            setrlimit(RLIMIT_CPU, &cpu_limit);
            setrlimit(RLIMIT_AS, &as_limit);

            if(execvp(cmd_args[0], cmd_args) == -1){
                fprintf(stderr, "En error occurred while running the command \"%s\"\n", cmd_args[0]);
                exit(101);
            }

            return 0;
        }

        // wait for child process to end
        int process_status;
        if(wait(&process_status) < 0){
            fprintf(stderr, "En error occurred while waiting for child precess to end\n");
            exit(101);
        }

        // stop timer
        stop_timer(&T);

        // check if the process stopped
        if(WIFSIGNALED(process_status)){
            fprintf(stderr, "En error occurred while running the command \"%s\": process terminated by %d\n", cmd_args[0], WTERMSIG(process_status));
            exit(102);
        }

        // check child process return something different then 0
        if(WIFEXITED(process_status) && WEXITSTATUS(process_status) != 0){
            fprintf(stderr, "Command \"%s\" ended with unexpected status %d\n", cmd_args[0], process_status);
            exit(103);
        }

        // display timer info
        display_time(&T, "Command resources usage");
    }

}
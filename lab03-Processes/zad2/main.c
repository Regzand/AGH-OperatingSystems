#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_ARGS 20

int main(int argc, char** args) {

    // check arguments count
    if (argc < 2) {
        fprintf(stderr, "Missing arguments!\nUsage: <path>\n");
        exit(1);
    }

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

        // execute cmd in child precess
        if(process_id == 0){

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
    }

}
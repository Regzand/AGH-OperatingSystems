#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define PIPE_INPUT      1
#define PIPE_OUTPUT     0

#define FIRST_CMD       1
#define LAST_CMD        2

#define LOGGING         0
#define LOG_LINES       1

#define MAX_ARGS        20
#define MAX_COMMANDS    10

// array for storing children pid
pid_t* pids;

// array for storing pipes
int pipes[MAX_COMMANDS][2];

// runs command
void execute_command(char* cmd, const int cmd_i, const int cmd_flag){

    // logging
    if(LOGGING){
        if(cmd_flag & FIRST_CMD)
            printf("\tFIRST\n");
        if(cmd_flag & LAST_CMD)
            printf("\tLAST\n");
        printf("\t%s\n", cmd);
    }

    // array for args
    char** args = malloc(sizeof(char*) * MAX_ARGS);
    int argc = 0;

    // pointer to differ strtok
    char* strtok_pointer;

    // reads first argument in command
    args[0] = strtok_r(cmd, " \n", &strtok_pointer);
    argc++;

    // if command is empty error
    if(args[0] == NULL){
        fprintf(stderr, "Missing command!\n");
        exit(10);
    }

    // parse rest of arguments
    while(argc < MAX_ARGS && (args[argc++] = strtok_r(NULL, " \n", &strtok_pointer)) != NULL);

    // set null at end
    args[argc-1] = NULL;

    // logging
    if(LOGGING)
        for(int i = 0; i < argc; i++)
            printf("\t\t%s\n", args[i]);

    // creates pipe
    pipe(pipes[cmd_i]);

    // create child process
    pids[cmd_i] = fork();
    if(pids[cmd_i] < 0){
        perror("En error occurred while creating process with fork()");
        exit(100);
    }

    // main process has nothing to do here anymore
    if(pids[cmd_i] > 0){
        close(pipes[cmd_i][PIPE_INPUT]);
        return;
    }


    // ========================================
    // child process section
    // ========================================

    // setup input (except for first command)
    if(!(cmd_flag & FIRST_CMD)) {
        dup2(pipes[cmd_i - 1][PIPE_OUTPUT], STDIN_FILENO);
        close(pipes[cmd_i - 1][PIPE_OUTPUT]);
    }

    // setup output (except for last command)
    if(!(cmd_flag & LAST_CMD)) {
        dup2(pipes[cmd_i][PIPE_INPUT], STDOUT_FILENO);
        close(pipes[cmd_i][PIPE_INPUT]);
    }

    // execute command
    execvp(args[0], args);

    // if this was reached there was an error
    perror("En error occurred while running command using execvp()");
    exit(100);

}

// runs one command after another
void execute_line(char* line){

    // print line info
    if(LOG_LINES) printf("\n\n\x1B[32m> %s\x1B[0m\n", line);

    // logging
    if(LOGGING) printf("%s\n", line);

    // array for commands
    char** cmds = malloc(sizeof(char*) * MAX_COMMANDS);
    int cmdc = 0;

    // pointer to differ strtok
    char* strtok_pointer;

    // reads first command in line
    cmds[0] = strtok_r(line, "|", &strtok_pointer);
    cmdc++;

    // if line is empty return
    if(cmds[0] == NULL)
        return;

    // parse rest of commands
    while(cmdc < MAX_COMMANDS && (cmds[cmdc++] = strtok_r(NULL, "|", &strtok_pointer)) != NULL);

    // fix last command
    if(cmds[cmdc-1] == NULL)
        cmdc--;

    // execute commands
    for(int i = 0; i < cmdc; i++)
        execute_command(cmds[i], i, (0 | (i == 0 ? FIRST_CMD : 0) | (i == cmdc-1 ? LAST_CMD : 0)));

    // wait for all commands to end
    for(int i = 0; i < cmdc; i++)
        waitpid(pids[i], NULL, 0);
}

// runs one line after another
int main(int argc, char** args) {

    // init array of pids
    pids = malloc(sizeof(pid_t) * MAX_COMMANDS);

    // check arguments count
    if (argc < 2) {
        fprintf(stderr, "Missing arguments!\nUsage: <path>\n");
        exit(1);
    }

    // opens file
    FILE* input = fopen(args[1], "r");
    if(input == NULL){
        perror("An error occurred while opening file with commands:");
        exit(2);
    }

    // reads file line by line
    char* line = NULL;
    size_t line_size = 0;
    while(getline(&line, &line_size, input) != -1)
        execute_line(line);

    // closes file
    fclose(input);

    return 0;
}
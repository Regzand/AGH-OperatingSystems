#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

char* script_file;

pid_t child_process_id = -1;
int waiting = 0;

void handle_SIGINT(int sig_no){

    // if child process is active kill it
    if(!waiting)
        kill(child_process_id, SIGKILL);

    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

void handle_SIGTSTP(int sig_no){

    // if program was waiting for SIGTSTP just continue
    if(waiting){
        waiting = 0;
        return;
    }

    // kill child process (instead of stopping loop)
    kill(child_process_id, SIGKILL);

    // prints info
    fprintf(stdout, "Oczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");

    // creates masks
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGTSTP);
    sigdelset(&mask, SIGINT);

    // waits for signal SIGINT or SIGTSTP
    waiting = 1;
    sigsuspend(&mask);
}

void run_script(){

    // creates child process
    pid_t process_id = fork();
    if(process_id < 0){
        fprintf(stderr, "En error occurred while creating process with fork()\n");
        exit(100);
    }

    // in child process runs script
    if(process_id == 0){

        // disables signal handling in child process
        signal(SIGTSTP, SIG_IGN);
        signal(SIGINT, SIG_IGN);

        // executes script
        if(execlp(script_file, script_file, NULL) == -1) {
            fprintf(stderr, "En error occurred while executing script\n");
            exit(200);
        }
    }

    // in parent process saves child process id so it can be later controlled
    if(process_id > 0) {
        child_process_id = process_id;
    }

}

int main(int argc, char** args){

    // checks arguments count
    if(argc < 2){
        fprintf(stderr, "Missing arguments!\nUsage: <script>\n");
        exit(1);
    }

    // set script file name
    script_file = args[1];

    // sets handler for SIGINT
    if(signal(SIGINT, handle_SIGINT) == SIG_ERR){
        fprintf(stderr, "En error occurred while setting handler for SIGINT");
        exit(2);
    }

    // creates sigaction struct
    struct sigaction sa_sigtstp;
    sa_sigtstp.sa_flags = 0;
    sa_sigtstp.sa_handler = handle_SIGTSTP;
    sigemptyset(&sa_sigtstp.sa_mask);

    // registers sigaction struct
    if(sigaction(SIGTSTP, &sa_sigtstp, NULL) == -1){
        fprintf(stderr, "En error occurred while setting handler for SIGTSTP");
        exit(3);
    }

    // main program loop
    while(1){
        run_script();

        // waits for any signal
        pause();
    }

}
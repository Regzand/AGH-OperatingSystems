#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

int waiting = 0;

void handle_SIGINT(int sig_no){
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

void handle_SIGTSTP(int sig_no){

    // if program was waiting for SIGTSTP just continue
    if(waiting){
        waiting = 0;
        return;
    }

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

void print_time(){

    // gets current time
    time_t current_time;
    current_time = time(NULL);

    // converts time to string in local format
    char* c_time_string;
    c_time_string = ctime(&current_time);

    // displays time
    printf("TIME: %s\n", c_time_string);
}

int main(int argc, char** args){

    // sets handler for SIGINT
    if(signal(SIGINT, handle_SIGINT) == SIG_ERR){
        fprintf(stderr, "En error occurred while setting handler for SIGINT");
        exit(1);
    }

    // creates sigaction struct
    struct sigaction sa_sigtstp;
    sa_sigtstp.sa_flags = 0;
    sa_sigtstp.sa_handler = handle_SIGTSTP;
    sigemptyset(&sa_sigtstp.sa_mask);

    // registers sigaction struct
    if(sigaction(SIGTSTP, &sa_sigtstp, NULL) == -1){
        fprintf(stderr, "En error occurred while setting handler for SIGTSTP");
        exit(2);
    }

    // main program loop
    while(1){
        print_time();
        sleep(1);
    }

}
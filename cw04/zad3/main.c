#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

const union sigval EMPTY_SIGVAL;    // used where empty sigval is needed

// =========================================================================
//  LOGGING
// =========================================================================
#define LOG_MAIN            1       // details about main process
#define LOG_CHILD           1       // details about child processes



// =========================================================================
//  COLORS
// =========================================================================
#define RED         "\x1B[31m"
#define GREEN       "\x1B[32m"
#define YELLOW      "\x1B[33m"
#define BLUE        "\x1B[34m"
#define MAGENTA     "\x1B[35m"
#define CYAN        "\x1B[36m"
#define WHITE       "\x1B[37m"
#define RESET       "\x1B[0m"



// =========================================================================
//  PROGRAM DATA
// =========================================================================
int L;                              // number of signals to send
int TYPE;                           // way of sending signals (1,2,3)

pid_t child_process_id;             // pid of created process

int counter_main_sent       = 0;
int counter_main_received   = 0;
int counter_child_sent      = 0;
int counter_child_received  = 0;



// =========================================================================
//  MAIN PROGRAM
// =========================================================================
int main(int argc, char** args);                                    // starts program and does initial settings

void main_type1();                                                  // runs program using method 1
void main_type2();                                                  // runs program using method 2
void main_type3();                                                  // runs program using method 3

void handle_type1(int sig_no);                                      // handles responses from child program
void handle_type2(int sig_no);                                      // handles responses from child program

void set_SIGINT_handler();                                          // sets SIGINT handler
void handle_SIGINT(int sig_no);                                     // handles SIGINT (Ctrl+C)

void create_child_process();                                        // creates child process

int MSG_SIGNAL;
int TER_SIGNAL;



// =========================================================================
//  CHILD PROGRAM
// =========================================================================
void child_main();                                                  // starts child program

void set_child_handlers();                                          // sets handlers for child

void handle_MSG_SIGNAL(int sig_no);                                    // handles MSG_SIGNAL - echo
void handle_TER_SIGNAL(int sig_no);                                    // handles TER_SIGNAL - exit



// =========================================================================
//  MAIN PROGRAM IMPLEMENTATION
// =========================================================================
int main(int argc, char** args) {

    // checks arguments count
    if (argc < 3) {
        fprintf(stderr, "Missing arguments!\nUsage: <L> <type>\n");
        exit(1);
    }

    // reads program arguments
    L = atoi(args[1]);
    TYPE = atoi(args[2]);

    // choose method
    switch (TYPE){
        case 1:
            MSG_SIGNAL = SIGUSR1;
            TER_SIGNAL = SIGUSR2;

            main_type1();
            break;
        case 2:
            MSG_SIGNAL = SIGUSR1;
            TER_SIGNAL = SIGUSR2;

            main_type2();
            break;
        case 3:
            MSG_SIGNAL = SIGRTMIN;
            TER_SIGNAL = SIGRTMAX;

            main_type1();
            break;
        default:
            fprintf(stderr, "Wrong type value!\nExpected: 1|2|3\n");
    }

    return 0;
}

void main_type1(){

    // creates child process and wait so it can start
    create_child_process();
    sleep(1);

    // sets SIGINT handler
    set_SIGINT_handler();

    // sets handler
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sigdelset(&sa.sa_mask, SIGINT);
    sa.sa_handler = handle_type1;
    if(sigaction(MSG_SIGNAL, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for MSG_SIGNAL");
        exit(400);
    }

    // sends MSG_SIGNAL L times
    for(int i = 0; i < L; i++){

        // sends signal
        kill(child_process_id, MSG_SIGNAL);
        counter_main_sent++;

        // logging
        if(LOG_MAIN) printf(BLUE "[MAIN] Signal MSG_SIGNAL sent\n" RESET);

    }

    // sends TER_SIGNAL
    kill(child_process_id, TER_SIGNAL);
    counter_main_sent++;

    // logging
    if(LOG_MAIN) printf(RED "[MAIN] Signal TER_SIGNAL sent\n" RESET);

    // display main info
    sleep(1);
    printf(GREEN "Main received %d signals and sent %d\n" RESET, counter_main_received, counter_main_sent);

    exit(0);
}

void main_type2(){

    // creates child process and wait so it can start
    create_child_process();
    sleep(1);

    // sets SIGINT handler
    set_SIGINT_handler();

    // sets handler
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sigdelset(&sa.sa_mask, SIGINT);
    sa.sa_handler = handle_type2;
    if(sigaction(MSG_SIGNAL, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for MSG_SIGNAL");
        exit(401);
    }

    // sends MSG_SIGNAL once
    kill(child_process_id, MSG_SIGNAL);
    counter_main_sent++;

    // logging
    if(LOG_MAIN) printf(BLUE "[MAIN] Signal MSG_SIGNAL sent\n" RESET);

    // creates process mask
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, MSG_SIGNAL);
    sigdelset(&mask, SIGINT);

    // waiting for response
    while(1)
        sigsuspend(&mask);

}

void create_child_process(){

    // creates child process
    child_process_id = fork();
    if(child_process_id < 0){
        perror("En error occurred while creating process with fork()");
        exit(100);
    }

    // starts process
    if(child_process_id == 0) {
        child_main();
        exit(666);      // should not happen
    }

    // logging
    if(LOG_MAIN) printf(RESET "[MAIN] Created process - PID: %d\n", child_process_id);

}

void handle_type1(int sig_no){

    // counting
    counter_main_received++;

    // logging
    if(LOG_MAIN) printf(CYAN "[MAIN] Signal MSG_SIGNAL received\n" RESET);

}

void handle_type2(int sig_no){

    // counting
    counter_main_received++;

    // logging
    if(LOG_MAIN) printf(CYAN "[MAIN] Signal MSG_SIGNAL received\n" RESET);

    // if all signals were sent end program
    if(counter_main_sent >= L){

        // terminate child process
        kill(child_process_id, TER_SIGNAL);

        // counting
        counter_main_sent++;

        // logging
        if(LOG_MAIN) printf(RED "[MAIN] Signal TER_SIGNAL sent\n" RESET);


        // display main info
        sleep(1);
        printf(GREEN "Main received %d signals and sent %d\n" RESET, counter_main_received, counter_main_sent);

        // end main process
        exit(0);
    }

    // send next signal
    kill(child_process_id, MSG_SIGNAL);

    // counting
    counter_main_sent++;

    // logging
    if(LOG_MAIN) printf(BLUE "[MAIN] Signal MSG_SIGNAL sent\n" RESET);
}

void set_SIGINT_handler(){

    // sets handler
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_handler = handle_SIGINT;
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for SIGINT");
        exit(402);
    }

}

void handle_SIGINT(int sig_no){

    // logging
    if(LOG_MAIN) printf(RED "[MAIN] Received SIGINT\n" RESET);

    // sends TER_SIGNAL
    kill(child_process_id, TER_SIGNAL);
    counter_main_sent++;

    // logging
    if(LOG_MAIN) printf(RED "[MAIN] Signal TER_SIGNAL sent\n" RESET);

    // display main info
    sleep(1);
    printf(GREEN "Main received %d signals and sent %d\n" RESET, counter_main_received, counter_main_sent);

    exit(0);

}

// =========================================================================
//  CHILD PROGRAM IMPLEMENTATION
// =========================================================================
void child_main(){

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Process started\n" RESET, getpid());

    // sets child handlers
    set_child_handlers();

    // creates process mask
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, MSG_SIGNAL);
    sigdelset(&mask, TER_SIGNAL);

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Waiting for signals\n" RESET, getpid());

    // waits for signals
    while(1)
        sigsuspend(&mask);
}

void set_child_handlers(){

    // creates sigaction used to set handlers
    struct sigaction sa;
    sigfillset(&sa.sa_mask);

    // sets handler for MSG_SIGNAL
    sa.sa_handler = handle_MSG_SIGNAL;
    if(sigaction(MSG_SIGNAL, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for MSG_SIGNAL");
        exit(301);
    }

    // sets handler for TER_SIGNAL
    sigdelset(&sa.sa_mask, MSG_SIGNAL);
    sa.sa_handler = handle_TER_SIGNAL;
    if(sigaction(TER_SIGNAL, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for TER_SIGNAL");
        exit(302);
    }

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Handlers ready\n" RESET, getpid());

}

void handle_MSG_SIGNAL(int sig_no){

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Received MSG_SIGNAL - echo signal\n" RESET, getpid());

    // counting
    counter_child_received++;

    // sending response
    kill(getppid(), MSG_SIGNAL);

    // counting
    counter_child_sent++;

}

void handle_TER_SIGNAL(int sig_no){

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Received TER_SIGNAL - termination signal\n" RESET, getpid());

    // counting
    counter_child_received++;

    // display child info
    printf(GREEN "Child received %d signals and sent %d\n" RESET, counter_child_received, counter_child_sent);

    // exit child process
    exit(0);

}

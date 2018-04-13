#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define _SIGUSR1 SIGRTMAX           // FIXME
const union sigval EMPTY_SIGVAL;    // used where empty sigval is needed

// =========================================================================
//  LOGGING
// =========================================================================
#define LOG_MAIN            1       // details about main process
#define LOG_CHILD           1       // details about child processes
#define LOG_CREATION        1       // info about creating processes (fork)
#define LOG_REQUESTS        1       // info about receiving requests (SIGUSR1)
#define LOG_PERMITS         1       // info about granting permits (SIGUSR2)
#define LOG_SIGNALS         1       // info about receiving runtime signals from processes
#define LOG_TERMINATIONS    1       // info about terminations of processes (SIGCHLD)



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
//  MAIN PROGRAM DATA
// =========================================================================
int N;                              // number of child processes to run
int K;                              // limit at which program will start accepting requests for processes
pid_t* processes_ids;               // array with child processes id's
int* processes_statuses;            // array with process statuses (0 - running, 1 - requested, 2 - granted, -1 - ended)
int main_status;                    // main process status (0 - ended, 1 - running, 2 - accept all)



// =========================================================================
//  MAIN PROGRAM
// =========================================================================
int main(int argc, char** args);                                    // starts program and does initial settings

void set_handlers();                                                // sets all handlers for main process
void create_child_processes();                                      // starts all child processes (N of them)

void handle_SIGINT(int sig_no);                                     // handles SIGINT  - Ctrl+C
void handle_SIGCHLD(int sig_no);                                    // handles SIGCHLD - End of child process
void handle_SIGUSR1(int sig_no, siginfo_t* info, void *ucontext);   // handles SIGUSR1 - Request
void handle_SIGRT(int sig_no, siginfo_t* info, void *ucontext);     // handles SIGINT  - Runtime signals

int get_process_number(int pid);                                    // returns number of process from given pid
int count_processes(int status);                                    // return number of processes that are in given state



// =========================================================================
//  CHILD PROGRAM DATA
// =========================================================================
int sleep_time;



// =========================================================================
//  CHILD PROGRAM
// =========================================================================
void child_main();                                                  // starts child program

void disable_handlers();                                            // disables handlers from main process
void set_child_handlers();                                          // sets handlers for child process

void handle_SIGUSR2(int sig_no);                                    // handles SIGUSR2 - response from main



// =========================================================================
//  MAIN PROGRAM IMPLEMENTATION
// =========================================================================
int main(int argc, char** args) {

    // checks arguments count
    if (argc < 3) {
        fprintf(stderr, "Missing arguments!\nUsage: <N> <K>\n");
        exit(1);
    }

    // reads program arguments
    N = atoi(args[1]);
    K = atoi(args[2]);

    // creates array for storing child processes
    processes_ids = malloc(sizeof(pid_t) * N);
    processes_statuses = malloc(sizeof(int) * N);

    // sets main status
    main_status = 1;

    // sets signal handlers
    set_handlers();

    // creates child processes
    create_child_processes();

    // wait for program to end
    while(main_status){
        sleep(1); // TODO
    }

}

void set_handlers(){

    // creates sigaction used to set handlers
    struct sigaction sa;
    sigfillset(&sa.sa_mask);

    // sets SIGINT handler - everything blocked
    sa.sa_handler = handle_SIGINT;
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for SIGINT");
        exit(300);
    }

    // sets SIGCHLD handler - passes SIGINT, using SA_NOCLDSTOP flag
    sa.sa_handler = handle_SIGCHLD;
    sigdelset(&sa.sa_mask, SIGINT);
    sa.sa_flags = SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for SIGCHLD");
        exit(301);
    }

    // sets SIGUSR1 handler - passes SIGINT, using SA_SIGINFO flag
    sa.sa_sigaction = handle_SIGUSR1;
    sa.sa_flags = SA_SIGINFO;
    if(sigaction(_SIGUSR1, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for SIGUSR1");
        exit(302);
    }

    // sets handler for all SIGRT signals - passes SIGINT, using SA_SIGINFO flag
    sa.sa_sigaction = handle_SIGRT;
    sa.sa_flags = SA_SIGINFO;
    for(int i = SIGRTMIN; i < SIGRTMAX; i++){
        if(sigaction(i, &sa, NULL) == -1) {
            perror("En error occurred while creating handler for SIGRT");
            exit(303);
        }
    }

    // logging
    if(LOG_MAIN) printf("[MAIN] Signal handlers ready\n");
}

void create_child_processes(){

    for(int i = 0; i < N; i++){

        // creates child process
        pid_t process_id = fork();
        if(process_id < 0){
            perror("En error occurred while creating process with fork()");
            exit(100);
        }

        // in child process starts its program
        if(process_id == 0){
            child_main();
            exit(666); // should never happen
        }

        // in main process saved process pid (and displays info)
        if(process_id > 0){
            processes_ids[i] = process_id;
            processes_statuses[i] = 0;

            if(LOG_CREATION) printf(GREEN "[MAIN] Started process #%d - PID: %d\n" RESET, i, process_id);
        }
    }

}

void handle_SIGINT(int sig_no){

    // block multiple executions
    if(!main_status) return;

    // logging
    if(LOG_MAIN) printf(RED "[MAIN] Received SIGINT\n" RESET);

    // kill all still alive processes
    for(int i = 0; i < N; i++){
        if(processes_statuses[i] != -1){
            kill(processes_ids[i], SIGQUIT);
            processes_statuses[i] = -1;
            if(LOG_MAIN) printf(RED "[MAIN] Killed process #%d - PID: %d \n" RESET, i, processes_ids[i]);
        }
    }

    // disable this handler and exit program
    main_status = 0;
    exit(0);

}

void handle_SIGCHLD(int sig_no){

    // gets terminated processes
    int process_id;
    int process_status;
    int process_number;
    while((process_id = waitpid(-1, &process_status, WNOHANG)) > 0) {

        // gets process number
        process_number = get_process_number(process_id);
        if(process_number == -1){
            perror("There is no process with such id");
            exit(1);
        }

        // sets process status
        processes_statuses[process_number] = -1;

        // displays info
        if(LOG_TERMINATIONS) printf(GREEN "[MAIN] Process #%d - PID: %d ended with status %d\n" RESET, process_number, process_id, WEXITSTATUS(process_status));
    }

    // check if all processes has ended
    if(count_processes(-1) == N)
        main_status = 0;

}

void handle_SIGUSR1(int sig_no, siginfo_t* info, void *ucontext){

    // gets process id
    int process_id = info -> si_pid;

    // gets process number
    int process_number = get_process_number(process_id);
    if(process_number == -1){
        perror("There is no process with such id");
        exit(1);
    }

    // set process as requested
    processes_statuses[process_number] = 1;

    // logging
    if(LOG_REQUESTS) printf(BLUE "[MAIN] Received request from #%d - PID: %d\n" RESET, process_number, process_id);

    // check if there is minimum K requests
    if(count_processes(1) >= K)
        main_status = 2;

    if(main_status == 2) {
        // accept all requests
        for(int i = 0; i < N; i++) {
            if(processes_statuses[i] == 1) {

                processes_statuses[i] = 2;
                kill(processes_ids[i], SIGUSR2);

                // logging
                if(LOG_PERMITS)
                    printf(MAGENTA "[MAIN] Sent approval to process #%d - PID: %d\n" RESET, i, processes_ids[i]);
            }
        }
    }
}

void handle_SIGRT(int sig_no, siginfo_t* info, void *ucontext){

    // gets process id
    int process_id = info -> si_pid;

    // gets process number
    int process_number = get_process_number(process_id);
    if(process_number == -1){
        perror("There is no process with such id");
        exit(1);
    }

    // logging
    if(LOG_SIGNALS) printf(YELLOW "[MAIN] Received signal %d from process #%d - PID: %d\n" RESET, sig_no, process_number, process_id);
}

int get_process_number(int pid){
    for(int i = 0; i < N; i++)
        if(processes_ids[i] == pid)
            return i;
    return -1;
}

int count_processes(int status){
    int res = 0;
    for(int i = 0; i < N; i++)
        if(processes_statuses[i] == status)
            res++;
    return res;
}



// =========================================================================
//  CHILD PROGRAM IMPLEMENTATION
// =========================================================================
void child_main(){

    // disables handlers from main
    disable_handlers();

    // registers child handlers
    set_child_handlers();

    // using pid as seed to ensure difference between processes
    srand(getpid());

    // random sleep time
    sleep_time = rand() % 11;

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Sleep for %ds\n" RESET, getpid(), sleep_time);

    // sleep process
    sleep(sleep_time);

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Wakes up\n" RESET, getpid());

    // sends request to parent process
    if(sigqueue(getppid(), _SIGUSR1, EMPTY_SIGVAL) == -1){
        perror("En error occurred while sending SIGUSR1 to parent process");
        exit(201);
    }

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Signal SIGUSR1 sent to parent, waiting for response\n" RESET, getpid());

    // waiting
    while(1)
        sleep(1);
}

void disable_handlers(){

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGQUIT);

    // sets mask
    if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1){
        fprintf(stderr, "En error occurred while setting mask for process %d", getpid());
        perror("");
        exit(200);
    }

}

void set_child_handlers(){

    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sigdelset(&sa.sa_mask, SIGQUIT);
    sa.sa_handler = handle_SIGUSR2;
    if(sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("En error occurred while creating handler for SIGUSR2");
        exit(400);
    }

}

void handle_SIGUSR2(int sig_no){

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Received approval\n" RESET, getpid());

    // generate random runtime signal
    int sig = rand() % (SIGRTMAX - SIGRTMIN) + SIGRTMIN;

    // send runtime signal
    if(sigqueue(getppid(), sig, EMPTY_SIGVAL) == -1){
        perror("En error occurred while sending SIGRT to parent process");
        exit(401);
    }

    // logging
    if(LOG_CHILD) printf(WHITE "[%d] Sent signal %d\n" RESET, getpid(), sig);

    // end process
    exit(sleep_time);
}

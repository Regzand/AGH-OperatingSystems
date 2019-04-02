#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "timing.h"

// starts timer
void start_timer(timer* T){
//    T -> time_start = clock();
    getrusage(T -> RUSAGE_FLAG, &(T -> usage_start));
}

// stops timer
void stop_timer(timer* T){
    getrusage(T -> RUSAGE_FLAG, &(T -> usage_end));
    //T -> time_end = clock();
}

double elapsed_time(struct timeval start, struct timeval end){
    return (((double) end.tv_sec) + (((double) end.tv_usec) / 1000000)) - (((double) start.tv_sec) + (((double) start.tv_usec) / 1000000));
}

// displays time
void display_time(timer* T, char* title){

    double user = elapsed_time(T -> usage_start.ru_utime, T -> usage_end.ru_utime);
    double system = elapsed_time(T -> usage_start.ru_stime, T -> usage_end.ru_stime);
//    double time = ((double) (T -> time_end) - (T -> time_start)) / CLOCKS_PER_SEC;

    printf("< User:     %lf\n< System:   %lf\n", user, system);
}

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "timing.h"

// starts timer
void start_timer(timer* T){
    gettimeofday(&(T -> time_start), NULL);
    getrusage(RUSAGE_SELF, &(T -> usage_start));
}

// stops timer
void stop_timer(timer* T){
    getrusage(RUSAGE_SELF, &(T -> usage_end));
    gettimeofday(&(T -> time_end), NULL);
}

double elapsed_time(struct timeval start, struct timeval end){
    return (((double) end.tv_sec) + (((double) end.tv_usec) / 1000000)) - (((double) start.tv_sec) + (((double) start.tv_usec) / 1000000));
}

// displays time
void display_time(timer* T){

    double user = elapsed_time(T -> usage_start.ru_utime, T -> usage_end.ru_utime);
    double system = elapsed_time(T -> usage_start.ru_stime, T -> usage_end.ru_stime);
    double time = elapsed_time(T -> time_start, T -> time_end);

    printf("\tReal:\t%lf\n\tUser:\t%lf\n\tSystem:\t%lf\n", time, user, system);
}

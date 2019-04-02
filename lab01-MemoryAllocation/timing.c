#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "timing.h"

// starts timer
void startTimer(timer* T){
    T -> time_start = clock();
    getrusage(RUSAGE_SELF, &(T -> usage_start));
}

// stops timer
void stopTimer(timer* T){
    getrusage(RUSAGE_SELF, &(T -> usage_end));
    T -> time_end = clock();
}

double elapsedTime(struct timeval start, struct timeval end){
    return (((double) end.tv_sec) + (((double) end.tv_usec) / 1000000)) - (((double) start.tv_sec) + (((double) start.tv_usec) / 1000000));
}

// displays time
void displayTime(timer* T, char* title){

    double user = elapsedTime(T -> usage_start.ru_utime, T -> usage_end.ru_utime);
    double system = elapsedTime(T -> usage_start.ru_stime, T -> usage_end.ru_stime);
    double time = ((double) T -> time_end - T -> time_start) / CLOCKS_PER_SEC;

    printf("[TIME][%s]\tT: %lf U: %lf S: %lf\n", title, time, user, system);
}

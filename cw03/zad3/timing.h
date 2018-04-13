#ifndef TIMING_H
#define TIMING_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct timer {
    int RUSAGE_FLAG;

    struct rusage usage_start;
    struct rusage usage_end;
    clock_t time_start;
    clock_t time_end;
} timer;

// starts timer
void start_timer(timer* T);

// stops timer
void stop_timer(timer* T);

// displays time
void display_time(timer* T, char* title);

#endif

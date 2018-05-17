#ifndef TIMING_H
#define TIMING_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct timer {
    struct rusage usage_start;
    struct rusage usage_end;
    struct timeval time_start;
    struct timeval time_end;
} timer;

// starts timer
void start_timer(timer* T);

// stops timer
void stop_timer(timer* T);

// displays time
void display_time(timer* T);

#endif

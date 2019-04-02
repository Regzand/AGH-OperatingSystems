#ifndef TIMING_H
#define TIMING_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct timer {
    struct rusage usage_start;
    struct rusage usage_end;
    clock_t time_start;
    clock_t time_end;
} timer;

// starts timer
void startTimer(timer* T);

// stops timer
void stopTimer(timer* T);

// displays time
void displayTime(timer* T, char* title);

#endif

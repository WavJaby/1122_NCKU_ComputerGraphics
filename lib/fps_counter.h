#ifndef __FPS_COUNTER_H__
#define __FPS_COUNTER_H__

#include <stdint.h>
#include <stdio.h>
#include <time.h>

int avgTimes = 5, frames, countAvg;
float fps, tick;

struct timespec tickStart = {0};

uint64_t getTimePass(struct timespec *start) {
    struct timespec end;
#ifdef _WIN32
    clock_gettime(CLOCK_MONOTONIC, &end);
#else
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
#endif
    uint64_t timePass = (end.tv_sec - start->tv_sec) * 1000000 + (end.tv_nsec - start->tv_nsec) / 1000;
    *start = end;

    return timePass;
}

void tickUpdate(void (*fpsUpdate)(float fps, float tick)) {
    countAvg++;
    // Do periodic frame rate calculation
    if (countAvg == avgTimes) {
        float time = getTimePass(&tickStart) / 1000000.0f;
        fps = (float)frames / time;
        tick = (float)avgTimes / time;
        fpsUpdate(fps, tick);
        frames = 0;
        countAvg = 0;
    }
}

void frameUpdate() {
    frames++;
}

#endif
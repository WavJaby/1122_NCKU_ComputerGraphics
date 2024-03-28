#ifndef __FPS_COUNTER_H__
#define __FPS_COUNTER_H__

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define FPS_COUNTER_AVG 10

int fpsCounter_frameCount;
float deltaTimeTick;

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
    static struct timespec tickStart = {0}, tickTime = {0};
    static int countAvg = 0;

    deltaTimeTick = (getTimePass(&tickTime) / 1000000.0f);
    countAvg++;
    // Do periodic frame rate calculation
    if (countAvg == FPS_COUNTER_AVG) {
        float time = getTimePass(&tickStart) / 1000000.0f;
        float fps = (float)fpsCounter_frameCount / time;
        float tick = (float)FPS_COUNTER_AVG / time;
        if (fpsUpdate)
            fpsUpdate(fps, tick);
        fpsCounter_frameCount = 0;
        countAvg = 0;
    }
}

void fpsCounterInit() {
    tickUpdate(NULL);
    deltaTimeTick = 0;
}

void frameUpdate() {
    fpsCounter_frameCount++;
}

#endif
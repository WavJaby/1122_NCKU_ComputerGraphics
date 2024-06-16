#ifndef __FPS_COUNTER_H__
#define __FPS_COUNTER_H__

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define FPS_UPDATE_INTERVAL 100000

int fpsCounter_frameCount, fpsCounter_tickCount;
float deltaTimeUpdate, deltaTime;

/**
 * @brief Calculate time pass, and update timespec 
 * @param start 
 * @return uint64_t (micro second)
 */
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
/**
 * @brief Get time interval
 * @param a
 * @param b
 * @return uint64_t (micro second)
 */
static inline uint64_t timeInterval(struct timespec *a, struct timespec *b) {
    return (b->tv_sec - a->tv_sec) * 1000000 + (b->tv_nsec - a->tv_nsec) / 1000;
}

void tickUpdate() {
    static struct timespec tickStart = {0}, tickTime = {0};

    deltaTimeUpdate = (getTimePass(&tickTime) / 1000000.0f);
    ++fpsCounter_tickCount;
}

void fpsCounterInit() {
    tickUpdate(NULL);
    deltaTimeUpdate = 0;
    deltaTime = 0;
}

void frameUpdate(void (*fpsUpdate)(float fps, float tick)) {
    static struct timespec renderTime = {0}, lastFpsUpdateTime = {0};

    deltaTime = (getTimePass(&renderTime) / 1000000.0f);
    ++fpsCounter_frameCount;
    
    float time;
    // Do periodic frame rate calculation
    if (fpsUpdate && (time = timeInterval(&lastFpsUpdateTime, &renderTime)) >= FPS_UPDATE_INTERVAL) {
        lastFpsUpdateTime = renderTime;
        float timeSec = time / 1000000.0f;
        fpsUpdate(fpsCounter_frameCount / timeSec, fpsCounter_tickCount / timeSec);
        fpsCounter_frameCount = 0;
        fpsCounter_tickCount = 0;
    }
}

#endif
#ifndef __FPS_COUNTER_H__
#define __FPS_COUNTER_H__

#include <stdint.h>
#include <stdio.h>
#include <time.h>

int avgTimes = 5, iFrames;
float fps;

struct timespec start = {0};

uint64_t getTimePass() {
    struct timespec end;
#ifdef _WIN32
    clock_gettime(CLOCK_MONOTONIC, &end);
#else
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
#endif
    uint64_t timePass = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    start = end;

    return timePass;
}

void frameUpdate(void (*fpsUpdate)(float)) {
    iFrames++;
    // Do periodic frame rate calculation
    if (iFrames == avgTimes) {
        fps = 5.0f / (getTimePass() / 1000000.0f);
        fpsUpdate(fps);
        // printf("%.2f fps\n", fps);
        // printf("%llu\n", getTimePass());

        iFrames = 0;
    }
}

#endif
#ifndef APP_FPS_H
#define APP_FPS_H

#include <stdint.h>

typedef struct{
    double delta_time;       // Time since last frame (seconds)
    double accumulator;      // For fixed-step simulation
    double render_fps;       // Measured render FPS
    double sim_steps_per_second;
    uint64_t last_time;      // Last performance counter
    uint64_t frames;         // Rendered frames this second
    uint64_t sim_steps;      // Simulation steps this second
    double freq;             // Cached SDL performance frequency
    double fps_timer;        // Time accumulator for FPS reporting
}Fps;

void fps_init(Fps *fps);
void fps_update(Fps *fps);

#endif // APP_FPS_H

#include "../app/fps.h"

#include <SDL.h>

void fps_init(Fps *fps) {
    fps->last_time = SDL_GetPerformanceCounter();
    fps->accumulator = 0.0;
    fps->frames = 0;
    fps->sim_steps = 0;
    fps->render_fps = 0.0;
    fps->sim_steps_per_second = 0.0;
    fps->freq = (double)SDL_GetPerformanceFrequency();
    fps->fps_timer = 0.0;
    fps->delta_time = 0.0;
}

void fps_update(Fps *fps) {
    uint64_t now = SDL_GetPerformanceCounter();
    fps->delta_time = (now - fps->last_time) / fps->freq;
    fps->last_time = now;

    fps->accumulator += fps->delta_time;
    fps->frames++;
    fps->fps_timer += fps->delta_time;

    if(fps->fps_timer >= 1.0) {
        fps->render_fps = (double)fps->frames / fps->fps_timer;
        fps->sim_steps_per_second = (double)fps->sim_steps / fps->fps_timer;
        fps->frames = 0;
        fps->sim_steps = 0;
        fps->fps_timer = 0.0;
    }
}

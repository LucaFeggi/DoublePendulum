#ifndef RENDERER_SDL_COLOR_H
#define RENDERER_SDL_COLOR_H

#include "../../app/render_data.h"

#include "SDL.h"

typedef struct{
    float sin0;
    float cos0;
    float sin1;
    float cos1;
}PendulumTrig;

void color_get_double_pendulum(
    const PendulumRenderData *pen,
    float max_ang_vel,
    const PendulumTrig *trig,
    SDL_Color color[2]
);

#endif // RENDERER_SDL_COLOR_H

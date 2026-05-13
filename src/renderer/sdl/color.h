#ifndef RENDERER_SDL_COLOR_H
#define RENDERER_SDL_COLOR_H

#include "../render_frame.h"

#include "SDL.h"

typedef struct{
    float sin0;
    float cos0;
    float sin1;
    float cos1;
}PendulumRenderTrig;

void color_get_double_pendulum(
    const PendulumRenderSample *pen,
    const float len[2],
    float max_ang_vel,
    const PendulumRenderTrig *trig,
    SDL_Color color[2]
);

#endif // RENDERER_SDL_COLOR_H

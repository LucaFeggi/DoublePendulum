#ifndef RENDERER_SDL_COLOR_H
#define RENDERER_SDL_COLOR_H

#include "../../app/render_data.h"

#include "SDL.h"

void color_get_double_pendulum(const PendulumRenderData *pen, double max_ang_vel, SDL_Color color[2]);

#endif // RENDERER_SDL_COLOR_H

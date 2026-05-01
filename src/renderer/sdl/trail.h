#ifndef RENDERER_SDL_TRAIL_H
#define RENDERER_SDL_TRAIL_H

#include <stdbool.h>

#include "SDL.h"

typedef struct {
    int x0;
    int y0;
    int x1;
    int y1;
    SDL_Color color;
} RenderLine;

typedef struct {
    SDL_Texture *texture;
    SDL_FPoint *last_tip;
    bool *has_last;
    int pendulum_count;
    int w;
    int h;
} TrailLayer;

bool trail_layer_init(TrailLayer *trail, int pendulum_count);
void trail_layer_quit(TrailLayer *trail);
bool trail_layer_update(TrailLayer *trail, SDL_Renderer *renderer, const RenderLine *rod_lines, int w, int h);
void trail_layer_draw(const TrailLayer *trail, SDL_Renderer *renderer);

#endif // RENDERER_SDL_TRAIL_H

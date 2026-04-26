#ifndef RENDERER_SDL_TRAIL_H
#define RENDERER_SDL_TRAIL_H

#include "../../config.h"

#include <stdbool.h>

#include "SDL.h"

typedef struct{
    int x0;
    int y0;
    int x1;
    int y1;
    SDL_Color color;
}RenderLine;

typedef struct{
    float x0;
    float y0;
    float x1;
    float y1;
    SDL_Color color;
}TrailSegment;

typedef struct{
    float last_x;
    float last_y;
    bool has_last;
#if TOTAL_TRAIL_SAMPLES > 1
    TrailSegment segments[TOTAL_TRAIL_SAMPLES - 1];
#endif
    int segment_count;
    int head;
}Trail;     // circular buffer for storing normalized trail segments

void trail_init(Trail *t);
void trail_add(Trail *t, int x, int y, SDL_Color color, int w, int h);
void trail_render(const Trail *t, int w, int h, SDL_Renderer *ptr);

#endif // RENDERER_SDL_TRAIL_H

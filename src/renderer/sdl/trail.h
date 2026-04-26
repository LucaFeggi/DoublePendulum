#ifndef RENDERER_SDL_TRAIL_H
#define RENDERER_SDL_TRAIL_H

#include "../../config.h"

#include "SDL.h"

typedef struct{
    int x0;
    int y0;
    int x1;
    int y1;
    SDL_Color color;
}RenderLine;

typedef struct{
    float samples[2 * TOTAL_TRAIL_SAMPLES];     // multiplying by 2 because this are coordinates (x,y)
    SDL_Color colors[TOTAL_TRAIL_SAMPLES];      // store the color at each sample
    int size;
    int head;
}Trail;     // circular buffer for storing trail in O(1)

void trail_init(Trail *t);
void trail_add(Trail *t, int x, int y, SDL_Color color, int w, int h);
int trail_write_line_commands(const Trail *t, int w, int h, RenderLine *lines, int capacity);
void trail_render(Trail *t, int w, int h, SDL_Renderer *ptr);

#endif // RENDERER_SDL_TRAIL_H

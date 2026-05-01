#ifndef RENDERER_SDL_LINE_BATCH_H
#define RENDERER_SDL_LINE_BATCH_H

#include <stdbool.h>

#include "SDL.h"

typedef struct {
    SDL_Vertex *vertices;
    int *indices;
    int vertex_count;
    int index_count;
    int line_capacity;
} LineBatch;

bool line_batch_init(LineBatch *batch, int line_capacity);
void line_batch_quit(LineBatch *batch);
void line_batch_reset(LineBatch *batch);
bool line_batch_add(LineBatch *batch, float x0, float y0, float x1, float y1, float width, SDL_Color color);
bool line_batch_draw(const LineBatch *batch, SDL_Renderer *renderer);

#endif // RENDERER_SDL_LINE_BATCH_H

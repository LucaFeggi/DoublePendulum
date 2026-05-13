#ifndef RENDERER_SDL_TRAIL_H
#define RENDERER_SDL_TRAIL_H

#include <stdbool.h>

#include "line_batch.h"
#include "SDL.h"

typedef struct {
    int x0;
    int y0;
    int x1;
    int y1;
    SDL_Color color;
} PreparedRodLine;

typedef struct {
    SDL_Texture *texture;
} TrailBucket;

typedef struct {
    TrailBucket *buckets;
    SDL_FPoint *last_tip;
    bool *has_last;
    LineBatch line_batch;
    float bucket_timer;
    int current_bucket;
    int bucket_count;
    int pendulum_count;
    int w;
    int h;
} TrailLayer;

bool trail_layer_init(TrailLayer *trail, int pendulum_count);
void trail_layer_quit(TrailLayer *trail);
bool trail_layer_resize(TrailLayer *trail, SDL_Renderer *renderer, int w, int h);
bool trail_layer_update(TrailLayer *trail, SDL_Renderer *renderer, const PreparedRodLine *rod_lines, float delta_time);
void trail_layer_draw(const TrailLayer *trail, SDL_Renderer *renderer);

#endif // RENDERER_SDL_TRAIL_H

#include "trail.h"

#include <stdint.h>

void trail_init(Trail *t){
    t->last_x = 0.0f;
    t->last_y = 0.0f;
    t->has_last = false;
    t->segment_count = 0;
    t->head = 0;
}

void trail_add(Trail *t, int x, int y, SDL_Color color, int w, int h) {
    if(!t || w <= 0 || h <= 0) {
        return;
    }

    float nx = (float)x / (float)w;
    float ny = (float)y / (float)h;

#if TOTAL_TRAIL_SAMPLES > 1
    if(t->has_last) {
        int idx;
        if(t->segment_count < TOTAL_TRAIL_SAMPLES - 1) {
            idx = (t->head + t->segment_count) % (TOTAL_TRAIL_SAMPLES - 1);
            t->segment_count++;
        }
        else {
            idx = t->head;
            t->head = (t->head + 1) % (TOTAL_TRAIL_SAMPLES - 1);
        }

        t->segments[idx].x0 = t->last_x;
        t->segments[idx].y0 = t->last_y;
        t->segments[idx].x1 = nx;
        t->segments[idx].y1 = ny;
        t->segments[idx].color = color;
    }
#else
    (void)color;
#endif

    t->last_x = nx;
    t->last_y = ny;
    t->has_last = true;
}

void trail_render(const Trail *t, int w, int h, SDL_Renderer *ptr){
#if TOTAL_TRAIL_SAMPLES > 1
    if(!t || !ptr || w <= 0 || h <= 0 || t->segment_count <= 0) {
        return;
    }

    for(int i = 0; i < t->segment_count; i++) {
        int idx = (t->head + i) % (TOTAL_TRAIL_SAMPLES - 1);
        const TrailSegment *seg = &t->segments[idx];

        float alpha_factor = (float)(i + 1) / (float)t->segment_count;
        uint8_t alpha = (uint8_t)(seg->color.a * alpha_factor);

        SDL_SetRenderDrawColor(ptr, seg->color.r, seg->color.g, seg->color.b, alpha);
        SDL_RenderDrawLine(
            ptr,
            (int)(seg->x0 * (float)w),
            (int)(seg->y0 * (float)h),
            (int)(seg->x1 * (float)w),
            (int)(seg->y1 * (float)h)
        );
    }
#else
    (void)t;
    (void)w;
    (void)h;
    (void)ptr;
#endif
}

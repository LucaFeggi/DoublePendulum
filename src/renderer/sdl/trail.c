#include "trail.h"
#include <stdio.h>
#include <stdint.h>

void trail_init(Trail *t){
    t->size = 0;
    t->head = 0;
}

void trail_add(Trail *t, int x, int y, SDL_Color color, int w, int h) {
    int idx;
    if(t->size < TOTAL_TRAIL_SAMPLES) {
        idx = (t->head + t->size) % TOTAL_TRAIL_SAMPLES;
        t->size++;
    }
    else {
        idx = t->head;
        t->head = (t->head + 1) % TOTAL_TRAIL_SAMPLES;
    }
    t->samples[2 * idx] = (float)x / w;
    t->samples[2 * idx + 1] = (float)y / h;
    t->colors[idx] = color;
}

void trail_render(Trail *t, int w, int h, SDL_Renderer *ptr){
    for(int i = 0; i < t->size - 1; i++) {
        int idx1 = (t->head + i) % TOTAL_TRAIL_SAMPLES;
        int idx2 = (t->head + i + 1) % TOTAL_TRAIL_SAMPLES;

        int x1 = t->samples[2 * idx1] * w;
        int y1 = t->samples[2 * idx1 + 1] * h;
        int x2 = t->samples[2 * idx2] * w;
        int y2 = t->samples[2 * idx2 + 1] * h;

        SDL_Color c1 = t->colors[idx1];
        float alphaFactor = (float)(i + 1) / t->size;
        uint8_t alpha1 = (uint8_t)(c1.a * alphaFactor);
        if(alpha1 > 255){
            alpha1 = 255;
        }

        SDL_SetRenderDrawColor(ptr, c1.r, c1.g, c1.b, alpha1);
        SDL_RenderDrawLine(ptr, x1, y1, x2, y2);
    }
}

#include "trail.h"
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

int trail_write_line_commands(const Trail *t, int w, int h, RenderLine *lines, int capacity) {
    if(!t || !lines || capacity <= 0 || t->size < 2) {
        return 0;
    }

    int line_count = 0;
    for(int i = 0; i < t->size - 1; i++) {
        if(line_count >= capacity) {
            break;
        }

        int idx1 = (t->head + i) % TOTAL_TRAIL_SAMPLES;
        int idx2 = (t->head + i + 1) % TOTAL_TRAIL_SAMPLES;

        int x1 = t->samples[2 * idx1] * w;
        int y1 = t->samples[2 * idx1 + 1] * h;
        int x2 = t->samples[2 * idx2] * w;
        int y2 = t->samples[2 * idx2 + 1] * h;

        SDL_Color c1 = t->colors[idx1];
        float alphaFactor = (float)(i + 1) / t->size;
        uint8_t alpha1 = (uint8_t)(c1.a * alphaFactor);

        lines[line_count].x0 = x1;
        lines[line_count].y0 = y1;
        lines[line_count].x1 = x2;
        lines[line_count].y1 = y2;
        lines[line_count].color.r = c1.r;
        lines[line_count].color.g = c1.g;
        lines[line_count].color.b = c1.b;
        lines[line_count].color.a = alpha1;
        line_count++;
    }

    return line_count;
}

void trail_render(Trail *t, int w, int h, SDL_Renderer *ptr){
#if TOTAL_TRAIL_SAMPLES > 1
    RenderLine lines[TOTAL_TRAIL_SAMPLES - 1];
    int line_count = trail_write_line_commands(t, w, h, lines, TOTAL_TRAIL_SAMPLES - 1);

    for(int i = 0; i < line_count; i++) {
        SDL_SetRenderDrawColor(ptr, lines[i].color.r, lines[i].color.g, lines[i].color.b, lines[i].color.a);
        SDL_RenderDrawLine(ptr, lines[i].x0, lines[i].y0, lines[i].x1, lines[i].y1);
    }
#else
    (void)t;
    (void)w;
    (void)h;
    (void)ptr;
#endif
}

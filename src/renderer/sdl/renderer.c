#include "../renderer.h"

#include "../../config.h"
#include "color.h"

#include <math.h>
#include <stdlib.h>

#define ROD_LINES_PER_PENDULUM 2
#if TOTAL_TRAIL_SAMPLES > 1
#define TRAIL_LINES_PER_PENDULUM (TOTAL_TRAIL_SAMPLES - 1)
#else
#define TRAIL_LINES_PER_PENDULUM 0
#endif

typedef struct {
    Renderer *renderer;
    RenderData *render_data;
    int w;
    int h;
    int center_x;
    int center_y;
    float max_rend_len;
} RendererPrepareJob;

static void renderer_prepare_range(RendererPrepareJob *job, int start_index, int end_index) {
    Renderer *renderer = job->renderer;
    RenderData *render_data = job->render_data;

    for(int i = start_index; i < end_index; i++) {
        double len0 = render_data->pen_data[i].len[0] / render_data->max_len * job->max_rend_len;
        double len1 = render_data->pen_data[i].len[1] / render_data->max_len * job->max_rend_len;

        int x0 = job->center_x + (int)(len0 * sin(render_data->pen_data[i].angle[0]));
        int y0 = job->center_y + (int)(len0 * cos(render_data->pen_data[i].angle[0]));

        int x1 = x0 + (int)(len1 * sin(render_data->pen_data[i].angle[1]));
        int y1 = y0 + (int)(len1 * cos(render_data->pen_data[i].angle[1]));

        SDL_Color color[2];
        color_get_double_pendulum(&render_data->pen_data[i], render_data->max_ang_vel, color);

        RenderLine *rod0 = &renderer->rod_lines[i * ROD_LINES_PER_PENDULUM];
        rod0->x0 = job->center_x;
        rod0->y0 = job->center_y;
        rod0->x1 = x0;
        rod0->y1 = y0;
        rod0->color = color[0];

        RenderLine *rod1 = &renderer->rod_lines[i * ROD_LINES_PER_PENDULUM + 1];
        rod1->x0 = x0;
        rod1->y0 = y0;
        rod1->x1 = x1;
        rod1->y1 = y1;
        rod1->color = color[1];

#if TRAIL
        trail_add(&renderer->trail[i], x1, y1, color[1], job->w, job->h);
#if TOTAL_TRAIL_SAMPLES > 1
        renderer->trail_line_counts[i] = trail_write_line_commands(
            &renderer->trail[i],
            job->w,
            job->h,
            &renderer->trail_lines[i * TRAIL_LINES_PER_PENDULUM],
            TRAIL_LINES_PER_PENDULUM
        );
#else
        renderer->trail_line_counts[i] = 0;
#endif
#endif
    }
}

static void renderer_prepare_job(void *context, int start_index, int end_index, int worker_id) {
    (void)worker_id;
    renderer_prepare_range((RendererPrepareJob *)context, start_index, end_index);
}

static void renderer_draw_line(SDL_Renderer *renderer, const RenderLine *line) {
    SDL_SetRenderDrawColor(renderer, line->color.r, line->color.g, line->color.b, line->color.a);
    SDL_RenderDrawLine(renderer, line->x0, line->y0, line->x1, line->y1);
}

bool renderer_init(Renderer *renderer, Window *window) {
    renderer->win_ptr = window->ptr;
    renderer->ptr = NULL;
    renderer->rod_lines = NULL;
    renderer->rod_line_count = 0;
#if TRAIL
    renderer->trail = NULL;
    renderer->trail_lines = NULL;
    renderer->trail_line_counts = NULL;
#endif

    renderer->ptr = SDL_CreateRenderer(window->ptr, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer->ptr == NULL){
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer->ptr, SDL_BLENDMODE_BLEND);

    renderer->rod_line_count = TOTAL_PENDULUMS * ROD_LINES_PER_PENDULUM;
    renderer->rod_lines = (RenderLine *)malloc((size_t)renderer->rod_line_count * sizeof(RenderLine));
    if(renderer->rod_lines == NULL) {
        SDL_Log("Could not allocate SDL rod line commands.");
        renderer_quit(renderer);
        return false;
    }

#if TRAIL
    renderer->trail = (Trail *)malloc(TOTAL_PENDULUMS * sizeof(Trail));
#if TOTAL_TRAIL_SAMPLES > 1
    renderer->trail_lines = (RenderLine *)malloc((size_t)TOTAL_PENDULUMS * TRAIL_LINES_PER_PENDULUM * sizeof(RenderLine));
#endif
    renderer->trail_line_counts = (int *)malloc(TOTAL_PENDULUMS * sizeof(int));
    if(renderer->trail == NULL || renderer->trail_line_counts == NULL
#if TOTAL_TRAIL_SAMPLES > 1
        || renderer->trail_lines == NULL
#endif
    ) {
        SDL_Log("Could not allocate SDL trail buffers.");
        renderer_quit(renderer);
        return false;
    }

    for(int i = 0; i < TOTAL_PENDULUMS; i++) {
        trail_init(&renderer->trail[i]); // Initialize each trail
        renderer->trail_line_counts[i] = 0;
    }
#endif
    return true;
}

void renderer_quit(Renderer *renderer) {
    free(renderer->rod_lines);
    renderer->rod_lines = NULL;
    renderer->rod_line_count = 0;

#if TRAIL
    free(renderer->trail);
    free(renderer->trail_lines);
    free(renderer->trail_line_counts);
    renderer->trail = NULL;
    renderer->trail_lines = NULL;
    renderer->trail_line_counts = NULL;
#endif

    if(renderer->ptr != NULL){
        SDL_DestroyRenderer(renderer->ptr);
    }
    renderer->ptr = NULL;
}

static void renderer_prepare(Renderer *renderer, RenderData *render_data, ThreadPool *threadpool){
    int w, h;
    SDL_GetWindowSize(renderer->win_ptr, &w, &h);

    RendererPrepareJob job = {
        .renderer = renderer,
        .render_data = render_data,
        .w = w,
        .h = h,
        .center_x = w / 2,
        .center_y = h / 2,
        .max_rend_len = (w < h) ? w / 5.0f : h / 5.0f
    };

    if(threadpool != NULL) {
        threadpool_parallel_for(threadpool, TOTAL_PENDULUMS, renderer_prepare_job, &job);
    }
    else {
        renderer_prepare_range(&job, 0, TOTAL_PENDULUMS);
    }
}

static void renderer_draw(Renderer *renderer){
    SDL_SetRenderDrawColor(renderer->ptr, 0x0, 0x0, 0x0, 0x0);
    SDL_RenderClear(renderer->ptr);

    for(int i = 0; i < TOTAL_PENDULUMS; i++) {
        renderer_draw_line(renderer->ptr, &renderer->rod_lines[i * ROD_LINES_PER_PENDULUM]);
        renderer_draw_line(renderer->ptr, &renderer->rod_lines[i * ROD_LINES_PER_PENDULUM + 1]);

#if TRAIL
#if TOTAL_TRAIL_SAMPLES > 1
        RenderLine *trail_lines = &renderer->trail_lines[i * TRAIL_LINES_PER_PENDULUM];
        int trail_line_count = renderer->trail_line_counts[i];
        for(int j = 0; j < trail_line_count; j++) {
            renderer_draw_line(renderer->ptr, &trail_lines[j]);
        }
#endif
#endif
    }

    SDL_RenderPresent(renderer->ptr);
}

void renderer_render(Renderer *renderer, RenderData *render_data, ThreadPool *threadpool) {
    renderer_prepare(renderer, render_data, threadpool);
    renderer_draw(renderer);
}

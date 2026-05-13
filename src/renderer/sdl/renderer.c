#include "../renderer.h"

#include "../../config/render_config.h"
#include "../../config/simulation_config.h"
#include "color.h"

#include <math.h>
#include <stdlib.h>

#define ROD_LINES_PER_PENDULUM 2

static bool renderer_trails_enabled(void) {
    return TRAIL != 0;
}

typedef struct {
    Renderer *renderer;
    const RenderData *render_data;
    int center_x;
    int center_y;
    float render_len[2];
} RendererPrepareJob;

static void renderer_prepare_range(const RendererPrepareJob *job, size_t start_index, size_t end_index) {
    Renderer *renderer = job->renderer;
    const RenderData *render_data = job->render_data;

    for(size_t i = start_index; i < end_index; i++) {
        float len0 = job->render_len[0];
        float len1 = job->render_len[1];
        PendulumRenderTrig trig = {
            .sin0 = sinf(render_data->pen_data[i].angle[0]),
            .cos0 = cosf(render_data->pen_data[i].angle[0]),
            .sin1 = sinf(render_data->pen_data[i].angle[1]),
            .cos1 = cosf(render_data->pen_data[i].angle[1])
        };

        int x0 = job->center_x + (int)(len0 * trig.sin0);
        int y0 = job->center_y + (int)(len0 * trig.cos0);

        int x1 = x0 + (int)(len1 * trig.sin1);
        int y1 = y0 + (int)(len1 * trig.cos1);

        SDL_Color color[2];
        color_get_double_pendulum(&render_data->pen_data[i], render_data->len, render_data->max_ang_vel, &trig, color);

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
    }
}

static void renderer_prepare_job(void *context, size_t start_index, size_t end_index, int worker_id) {
    (void)worker_id;
    renderer_prepare_range((const RendererPrepareJob *)context, start_index, end_index);
}

bool renderer_init(Renderer *renderer, const Window *window) {
    renderer->win_ptr = window->ptr;
    renderer->ptr = NULL;
    renderer->rod_lines = NULL;
    line_batch_init(&renderer->rod_batch, 0);
    renderer->trail_enabled = renderer_trails_enabled();
    trail_layer_init(&renderer->trail, 0);

    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    if(renderer->trail_enabled) {
        renderer_flags |= SDL_RENDERER_TARGETTEXTURE;
    }

    renderer->ptr = SDL_CreateRenderer(
        window->ptr,
        -1,
        renderer_flags
    );
    if(renderer->ptr == NULL) {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer->ptr, SDL_BLENDMODE_BLEND);

    renderer->rod_lines = (RenderLine *)malloc((size_t)TOTAL_PENDULUMS * ROD_LINES_PER_PENDULUM * sizeof(RenderLine));
    if(renderer->rod_lines == NULL) {
        SDL_Log("Could not allocate SDL rod line commands.");
        renderer_quit(renderer);
        return false;
    }

    if(!line_batch_init(&renderer->rod_batch, TOTAL_PENDULUMS * ROD_LINES_PER_PENDULUM)) {
        SDL_Log("Could not allocate SDL rod geometry batch.");
        renderer_quit(renderer);
        return false;
    }

    if(renderer->trail_enabled && !trail_layer_init(&renderer->trail, TOTAL_PENDULUMS)) {
        SDL_Log("Could not allocate SDL trail buffers.");
        renderer_quit(renderer);
        return false;
    }

    return true;
}

void renderer_quit(Renderer *renderer) {
    if(!renderer) {
        return;
    }

    free(renderer->rod_lines);
    renderer->rod_lines = NULL;
    line_batch_quit(&renderer->rod_batch);

    trail_layer_quit(&renderer->trail);
    renderer->trail_enabled = false;

    if(renderer->ptr != NULL) {
        SDL_DestroyRenderer(renderer->ptr);
        renderer->ptr = NULL;
    }

    renderer->win_ptr = NULL;
}

static void renderer_prepare(Renderer *renderer, const RenderData *render_data, ThreadPool *threadpool, int w, int h) {
    RendererPrepareJob job = {
        .renderer = renderer,
        .render_data = render_data,
        .center_x = w / 2,
        .center_y = h / 2
    };

    float max_rend_len = (w < h) ? w / 5.0f : h / 5.0f;
    float render_scale = render_data->max_len > 0.0f
        ? max_rend_len / render_data->max_len
        : 0.0f;
    job.render_len[0] = render_data->len[0] * render_scale;
    job.render_len[1] = render_data->len[1] * render_scale;

    if(threadpool != NULL) {
        threadpool_parallel_for(threadpool, (size_t)TOTAL_PENDULUMS, renderer_prepare_job, &job);
    }
    else {
        renderer_prepare_range(&job, (size_t)0, (size_t)TOTAL_PENDULUMS);
    }
}

static bool renderer_draw_rods(Renderer *renderer) {
    line_batch_reset(&renderer->rod_batch);

    for(int i = 0; i < TOTAL_PENDULUMS * ROD_LINES_PER_PENDULUM; ++i) {
        const RenderLine *line = &renderer->rod_lines[i];
        if(!line_batch_add(
            &renderer->rod_batch,
            (float)line->x0,
            (float)line->y0,
            (float)line->x1,
            (float)line->y1,
            ROD_WIDTH_PIXELS,
            line->color
        )) {
            SDL_Log("Could not add rod line to SDL geometry batch.");
            return false;
        }
    }

    if(!line_batch_draw(&renderer->rod_batch, renderer->ptr)) {
        SDL_Log("Could not draw SDL rod geometry batch: %s", SDL_GetError());
        return false;
    }

    return true;
}

static void renderer_draw(Renderer *renderer, int w, int h, float delta_time) {
    bool render_trails = renderer->trail_enabled;
    if(render_trails) {
        trail_layer_update(&renderer->trail, renderer->ptr, renderer->rod_lines, w, h, delta_time);
    }

    SDL_SetRenderTarget(renderer->ptr, NULL);
    SDL_SetRenderDrawBlendMode(renderer->ptr, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer->ptr, 0x0, 0x0, 0x0, 0xff);
    SDL_RenderClear(renderer->ptr);

    if(render_trails) {
        trail_layer_draw(&renderer->trail, renderer->ptr);
    }

    renderer_draw_rods(renderer);

    SDL_RenderPresent(renderer->ptr);
}

void renderer_render(Renderer *renderer, const RenderData *render_data, ThreadPool *threadpool, int w, int h, float delta_time) {
    renderer_prepare(renderer, render_data, threadpool, w, h);
    renderer_draw(renderer, w, h, delta_time);
}

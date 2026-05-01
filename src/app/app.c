#include "app.h"

#include "../config/config.h"
#include "input.h"

#include <SDL.h>
#include <SDL_cpuinfo.h>

static int app_get_worker_threads(void) {
    int cpu_count = SDL_GetCPUCount();
    return cpu_count > 1 ? cpu_count - 1 : 1;
}

static bool app_should_use_threadpool(void) {
    return TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD;
}

static ThreadPool *app_get_threadpool(App *app) {
    return app->threadpool_enabled ? &app->threadpool : NULL;
}

static int app_consume_simulation_steps(Fps *fps) {
    int steps = (int)(fps->accumulator / SIMULATION_DT);

    if(steps > MAX_SIMULATION_STEPS_PER_FRAME) {
        steps = MAX_SIMULATION_STEPS_PER_FRAME;
        fps->accumulator = 0.0;
    }
    else {
        fps->accumulator -= (double)steps * SIMULATION_DT;
    }

    return steps;
}

bool app_init(App *app) {
    app->threadpool_enabled = false;

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    fps_init(&app->fps);

    int worker_threads = app_get_worker_threads();
    if(app_should_use_threadpool()) {
        if(!threadpool_init(&app->threadpool, worker_threads)) {
            SDL_Log("ERROR: Failed to initialize threadpool.");
            goto fail_threadpool;
        }
        app->threadpool_enabled = true;
    }
    else {
        worker_threads = 0;
    }

    bool simulation_success = PENDULUM_INIT_MODE
        ? simulation_init_custom(&app->simulation, worker_threads)
        : simulation_init_default(&app->simulation, worker_threads);

    if(!simulation_success) goto fail_simulation;
    if(!window_init(&app->window)) goto fail_window;
    if(!render_data_init(&app->render_data, &app->simulation)) goto fail_render_data;
    if(!renderer_init(&app->renderer, &app->window)) goto fail_renderer;

    window_show(&app->window);
    return true;

fail_renderer:
    render_data_quit(&app->render_data);
fail_render_data:
    window_quit(&app->window);
fail_window:
    simulation_quit(&app->simulation);
fail_simulation:
    if(app->threadpool_enabled) {
        threadpool_quit(&app->threadpool);
        app->threadpool_enabled = false;
    }
fail_threadpool:
    SDL_Quit();

    return false;
}

void app_run(App *app) {
    bool quit = false;

    while(true) {
        input_poll(&quit);
        if(quit) {
            return;
        }

        fps_update(&app->fps);
        app->fps.accumulator += app->fps.delta_time * SIMULATION_TIME_SCALE;

        int steps = app_consume_simulation_steps(&app->fps);
        if(steps > 0) {
            simulation_update_steps(&app->simulation, app_get_threadpool(app), steps);
            app->fps.sim_steps += (uint64_t)steps;
        }

        int w;
        int h;
        window_update_title(&app->window, app->fps.delta_time, app->fps.render_fps, app->fps.sim_steps_per_second);
        if(window_get_render_size(&app->window, &w, &h)) {
            render_data_pack(&app->render_data, &app->simulation);
            renderer_render(&app->renderer, &app->render_data, app_get_threadpool(app), w, h);

            if(window_is_minimized(&app->window)) {
                SDL_Delay(1);
            }
        }
        else {
            SDL_Delay(1);
        }
    }
}

void app_quit(App *app) {
    renderer_quit(&app->renderer);
    render_data_quit(&app->render_data);
    window_quit(&app->window);
    simulation_quit(&app->simulation);

    if(app->threadpool_enabled) {
        threadpool_quit(&app->threadpool);
        app->threadpool_enabled = false;
    }

    SDL_Quit();
}

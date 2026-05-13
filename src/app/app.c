#include "app.h"

#include "../config/app_config.h"
#include "../config/simulation_config.h"
#include "input.h"

#include <SDL.h>
#include <SDL_cpuinfo.h>

#include <stdlib.h>

typedef struct {
    Simulation *simulation;
    int steps;
    double *thread_max_ang_vel;
    int thread_max_capacity;
} AppSimulationUpdateJob;

static int app_get_worker_threads(void) {
    int cpu_count = SDL_GetCPUCount();
    int max_worker_threads = cpu_count > 1 ? cpu_count - 1 : 1;
    size_t pendulum_count = (size_t)TOTAL_PENDULUMS;
    size_t min_items = (size_t)THREADPOOL_MIN_ITEMS_PER_JOB;
    size_t useful_workers = pendulum_count / min_items + ((pendulum_count % min_items) != 0);

    if(useful_workers < 1) {
        useful_workers = 1;
    }
    if(useful_workers > (size_t)max_worker_threads) {
        useful_workers = (size_t)max_worker_threads;
    }

    return (int)useful_workers;
}

static ThreadPool *app_get_threadpool(App *app) {
    return app->threadpool_enabled ? &app->threadpool : NULL;
}

static bool app_init_thread_scratch(App *app, int worker_threads) {
    app->thread_max_ang_vel = NULL;
    app->thread_max_capacity = 0;

    if(worker_threads <= 0) {
        return true;
    }

    int capacity = worker_threads;
    app->thread_max_ang_vel = (double *)calloc((size_t)capacity, sizeof(double));
    if(!app->thread_max_ang_vel) {
        return false;
    }

    app->thread_max_capacity = capacity;
    return true;
}

static void app_free_thread_scratch(App *app) {
    free(app->thread_max_ang_vel);
    app->thread_max_ang_vel = NULL;
    app->thread_max_capacity = 0;
}

static void app_simulation_update_job(void *context, size_t start_index, size_t end_index, int worker_id) {
    AppSimulationUpdateJob *job = (AppSimulationUpdateJob *)context;
    double max_ang_vel = simulation_update_range(job->simulation, start_index, end_index, job->steps);

    if(worker_id >= 0 && worker_id < job->thread_max_capacity) {
        job->thread_max_ang_vel[worker_id] = max_ang_vel;
    }
}

static double app_reduce_max_ang_vel(const double *max_ang_vel_by_job, int count) {
    double max_ang_vel = 0.0;

    for(int i = 0; i < count; ++i) {
        if(max_ang_vel_by_job[i] > max_ang_vel) {
            max_ang_vel = max_ang_vel_by_job[i];
        }
    }

    return max_ang_vel;
}

static void app_clear_thread_scratch(double *values, int count) {
    for(int i = 0; i < count; ++i) {
        values[i] = 0.0;
    }
}

static bool app_can_parallelize_simulation(const App *app) {
    return app->threadpool_enabled
        && app->thread_max_ang_vel != NULL;
}

static void app_update_simulation_steps(App *app, int steps) {
    if(steps <= 0) {
        return;
    }

    if(!app_can_parallelize_simulation(app)) {
        app->current_max_ang_vel = simulation_update_steps(&app->simulation, steps);
        return;
    }

    AppSimulationUpdateJob job = {
        .simulation = &app->simulation,
        .steps = steps,
        .thread_max_ang_vel = app->thread_max_ang_vel,
        .thread_max_capacity = app->thread_max_capacity
    };

    app_clear_thread_scratch(app->thread_max_ang_vel, app->thread_max_capacity);

    int active_jobs = threadpool_parallel_for(
        &app->threadpool,
        simulation_get_count(&app->simulation),
        app_simulation_update_job,
        &job
    );

    if(active_jobs > 0) {
        app->current_max_ang_vel = app_reduce_max_ang_vel(app->thread_max_ang_vel, app->thread_max_capacity);
    }
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
    app->current_max_ang_vel = 0.0;
    app->thread_max_ang_vel = NULL;
    app->thread_max_capacity = 0;

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    fps_init(&app->fps);

    int worker_threads = app_get_worker_threads();
    if(worker_threads > 0) {
        if(!threadpool_init(&app->threadpool, worker_threads)) {
            SDL_Log("ERROR: Failed to initialize threadpool.");
            goto fail_threadpool;
        }
        app->threadpool_enabled = true;

        if(!app_init_thread_scratch(app, worker_threads)) {
            SDL_Log("ERROR: Failed to allocate thread-local angular velocity buffer.");
            goto fail_thread_scratch;
        }
    }

    bool simulation_success = PENDULUM_INIT_MODE
        ? simulation_init_custom(&app->simulation)
        : simulation_init_default(&app->simulation);

    if(!simulation_success) {
        SDL_Log("ERROR: Failed to initialize simulation.");
        goto fail_simulation;
    }
    if(!window_init(&app->window)) goto fail_window;
    if(!render_data_init(&app->render_data, &app->simulation)) goto fail_render_data;
    app->current_max_ang_vel = (double)app->render_data.max_ang_vel;
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
    app_free_thread_scratch(app);
fail_thread_scratch:
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

        if(window_is_minimized(&app->window)) {
            app->fps.accumulator = 0.0;
            SDL_Delay(16);
            continue;
        }

        app->fps.accumulator += app->fps.delta_time * SIMULATION_TIME_SCALE;

        int steps = app_consume_simulation_steps(&app->fps);
        if(steps > 0) {
            app_update_simulation_steps(app, steps);
            app->fps.sim_steps += (uint64_t)steps;
        }

        int w;
        int h;
        window_update_title(&app->window, app->fps.delta_time, app->fps.render_fps, app->fps.sim_steps_per_second);

        if(window_get_render_size(&app->window, &w, &h)) {
            render_data_pack(&app->render_data, &app->simulation, (float)app->current_max_ang_vel, (float)app->fps.delta_time);
            renderer_render(&app->renderer, &app->render_data, app_get_threadpool(app), w, h, (float)app->fps.delta_time);
        }
        else {
            SDL_Delay(16);
        }
    }
}


void app_quit(App *app) {
    renderer_quit(&app->renderer);
    render_data_quit(&app->render_data);
    window_quit(&app->window);
    simulation_quit(&app->simulation);
    app_free_thread_scratch(app);

    if(app->threadpool_enabled) {
        threadpool_quit(&app->threadpool);
        app->threadpool_enabled = false;
    }

    SDL_Quit();
}

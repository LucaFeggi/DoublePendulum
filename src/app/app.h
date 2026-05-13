#ifndef APP_APP_H
#define APP_APP_H

#include "fps.h"
#include "window.h"
#include "render_data.h"

#include "../renderer/renderer.h"
#include "../simulation/simulation.h"
#include "../utils/threadpool.h"

#include <stdbool.h>

typedef struct {
    Fps fps;
    ThreadPool threadpool;
    bool threadpool_enabled;
    double current_max_ang_vel;
    double *thread_max_ang_vel;
    int thread_max_capacity;
    Simulation simulation;
    Window window;
    RenderData render_data;
    Renderer renderer;
} App;

bool app_init(App *app);
void app_run(App *app);
void app_quit(App *app);

#endif // APP_APP_H

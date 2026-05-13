#ifndef APP_APP_H
#define APP_APP_H

#include "fps.h"
#include "window.h"

#include "../renderer/render_frame.h"
#include "../renderer/renderer.h"
#include "../simulation/simulation.h"
#include "../utils/threadpool.h"

#include <stdbool.h>

typedef struct {
    Fps fps;
    ThreadPool threadpool;
    bool threadpool_enabled;
    double *thread_max_ang_vel;
    int thread_max_capacity;
    Simulation simulation;
    Window window;
    RenderFrame render_frame;
    Renderer renderer;
} App;

bool app_init(App *app);
void app_run(App *app);
void app_quit(App *app);

#endif // APP_APP_H

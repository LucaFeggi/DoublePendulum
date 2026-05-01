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
    Simulation simulation;
    Window window;
    RenderData render_data;
    Renderer renderer;
} App;

bool app_init(App *app);
void app_run(App *app);
void app_quit(App *app);

#endif // APP_APP_H

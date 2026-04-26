#include "app.h"

#include "../config.h"
#include "input.h"

#include <SDL.h>
#include <SDL_cpuinfo.h>

static int app_get_worker_threads(void) {
	int cpu_count = SDL_GetCPUCount();
	return cpu_count > 1 ? cpu_count - 1 : 1;
}

bool app_init(App *app) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    fps_init(&app->fps);

	int worker_threads = app_get_worker_threads();
	bool simulation_success = PENDULUM_INIT_MODE
		? simulation_init_custom(&app->simulation, worker_threads)
		: simulation_init_default(&app->simulation, worker_threads);

	if(!simulation_success) goto fail_simulation;
    if(!window_init(&app->window)) goto fail_window;
	if(!render_data_init(&app->render_data, app->simulation.max_len)) goto fail_render_data;
	if(!renderer_init(&app->renderer, &app->window)) goto fail_renderer;
    window_show(&app->window);  // showing window only when everything is initialized
    return true;

fail_renderer:
	render_data_quit(&app->render_data);
fail_render_data:
	window_quit(&app->window);
fail_window:
    simulation_quit(&app->simulation);
fail_simulation:
    SDL_Quit();

    return false;
}

 void app_run(App *app){
	bool quit = false;
	while(true){
		input_poll(&quit);
		if(quit){
			return;
		}
		fps_update(&app->fps);	
		while(app->fps.accumulator >= DT){		// Fixed-step simulation update
			simulation_update(&app->simulation);
			app->fps.sim_steps++;
			app->fps.accumulator -= DT;
		}	
		if(!window_is_minimized(&app->window)){		// rendering only if window is not minimized
			window_update_title(&app->window, app->fps.delta_time, app->fps.render_fps, app->fps.sim_steps_per_second);
			render_data_pack(&app->render_data, &app->simulation);
			renderer_render(&app->renderer, &app->render_data, simulation_get_threadpool(&app->simulation));
		}
	}
}

void app_quit(App *app){
	renderer_quit(&app->renderer);
	render_data_quit(&app->render_data);
	window_quit(&app->window);
	simulation_quit(&app->simulation);
	SDL_Quit();
}

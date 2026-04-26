#include "app.h"

#include "../config.h"
#include "input.h"

#include <SDL.h>

bool app_init(App *app) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    fps_init(&app->fps);

    bool simulation_success;
#if PENDULUM_INIT_MODE
	simulation_success = simulation_init_custom(&app->simulation);
#else
	simulation_success = simulation_init_default(&app->simulation);
#endif

    if(!simulation_success) goto fail_simulation;
    if(!window_init(&app->window)) goto fail_window;
	if(!render_data_init(&app->render_data, app->simulation.max_len)) goto fail_render_data;
#if RENDER_MODE
//    if(!vk_renderer_init(&app->renderer, &app->window)) goto fail_renderer;
#else
	if(!renderer_sdl_init(&app->renderer_sdl, &app->window)) goto fail_renderer;
#endif
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
			window_update_title(&app->window, app->fps.delta_time, app->fps.render_fps, app->fps.sim_fps);
			render_data_pack(&app->render_data, &app->simulation);
#if RENDER_MODE

#else
			renderer_sdl_render(&app->renderer_sdl, &app->render_data);
#endif
		//	vk_renderer_render(&app->renderer, &app->window);
		}
	}
}

void app_quit(App *app){
#if RENDER_MODE
	//	vk_renderer_free(&app->renderer);
#else
	renderer_sdl_quit(&app->renderer_sdl);
#endif
	render_data_quit(&app->render_data);
	window_quit(&app->window);
	simulation_quit(&app->simulation);
	SDL_Quit();
}
#ifndef APP_H
#define APP_H

#include "fps.h"
#include "window.h"
#include "render_data.h"

#include "../simulation/simulation.h"
#include "../renderer/sdl/renderer_sdl.h"
//#include "../renderer/vk_renderer.h"

#include <stdbool.h>

typedef struct{
	Fps fps;
	Simulation simulation;
	Window window;
	RenderData render_data;
	RendererSDL renderer_sdl;
	
//	VkRenderer renderer;
}App;

bool app_init(App *app);
void app_run(App *app);
void app_quit(App *app);

#endif
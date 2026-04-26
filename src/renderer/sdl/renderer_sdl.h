#ifndef RENDERER_SDL_H
#define RENDERER_SDL_H

#include "../../app/window.h"
#include "../../app/render_data.h"
#include "trail.h"

#include "SDL.h"

typedef struct {
	SDL_Window *win_ptr;	//owned by Window class
	SDL_Renderer *ptr;
#ifdef TRAIL
	Trail *trail;
#endif
}RendererSDL;

bool renderer_sdl_init(RendererSDL *renderer_sdl, Window *window);
void renderer_sdl_quit(RendererSDL *renderer_sdl);
void renderer_sdl_render(RendererSDL *renderer_sdl, RenderData *render_data);

#endif // !RENDERER_SDL_H
#ifndef RENDERER_SDL_RENDERER_BACKEND_H
#define RENDERER_SDL_RENDERER_BACKEND_H

#include "trail.h"

#include "SDL.h"

typedef struct {
	SDL_Window *win_ptr;	//owned by Window class
	SDL_Renderer *ptr;
    RenderLine *rod_lines;
	Trail *trail;
}Renderer;

#endif // RENDERER_SDL_RENDERER_BACKEND_H

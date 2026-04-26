#ifndef RENDERER_SDL_RENDERER_BACKEND_H
#define RENDERER_SDL_RENDERER_BACKEND_H

#include "../../config.h"
#include "trail.h"

#include "SDL.h"

typedef struct {
	SDL_Window *win_ptr;	//owned by Window class
	SDL_Renderer *ptr;
    RenderLine *rod_lines;
    int rod_line_count;
#if TRAIL
	Trail *trail;
    RenderLine *trail_lines;
    int *trail_line_counts;
#endif
}Renderer;

#endif // RENDERER_SDL_RENDERER_BACKEND_H

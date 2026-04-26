#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <stdbool.h>

#include <SDL.h>

typedef struct{
    SDL_Window *ptr;
    double title_timer;  // accumulates time for FPS title update
}Window;

bool window_init(Window *window);
void window_quit(Window *window);
void window_show(const Window *window);
bool window_is_minimized(const Window *window);
void window_update_title(Window *window, double delta_time, double render_fps, double sim_steps_per_second);

#endif // APP_WINDOW_H

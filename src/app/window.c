#include "window.h"

#include <stdio.h>

bool window_init(Window *window){

    SDL_DisplayMode dm;
    int w, h;
    if(SDL_GetCurrentDisplayMode(0, &dm) != 0) {
        SDL_Log("Could not get display mode: %s", SDL_GetError());
        w = 1280;
        h = 720;
    }
    else {
        w = (int)(dm.w * 0.667f);
        h = (int)(dm.h * 0.667f);
    }
    window->ptr = SDL_CreateWindow(
        "Double pendulum | Simulation steps/s: 0 | Renderer FPS: 0 |",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_HIDDEN
    );
    if(window->ptr == NULL) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return false;
    }
    SDL_SetWindowMinimumSize(window->ptr, w / 5, h / 5);

    SDL_Surface *icon = SDL_LoadBMP("assets/icon/icon.bmp");
    if(icon != NULL) {
        SDL_SetWindowIcon(window->ptr, icon);
        SDL_FreeSurface(icon);
    }
    else {
        SDL_Log("Warning: Could not load window icon 'assets/icon/icon.bmp': %s", SDL_GetError());
    }

    window->title_timer = 0.0;

    return true;
}

void window_quit(Window *window){
    if(window->ptr != NULL) {
        SDL_DestroyWindow(window->ptr);
        window->ptr = NULL;
    }
}

void window_show(const Window *window){
    SDL_ShowWindow(window->ptr);
}

bool window_is_minimized(const Window *window){
    return (SDL_GetWindowFlags(window->ptr) & SDL_WINDOW_MINIMIZED) != 0;
}

bool window_get_render_size(const Window *window, int *w, int *h) {
    SDL_GetWindowSize(window->ptr, w, h);
    return *w > 0 && *h > 0;
}

void window_update_title(Window *window, double delta_time, double render_fps, double sim_steps_per_second){
    window->title_timer += delta_time;
    if(window->title_timer >= 1.0) {
        char title[80];
        snprintf(
            title,
            sizeof(title),
            "Double pendulum | Simulation steps/s: %d | Renderer FPS: %d |",
            (int)(sim_steps_per_second + 0.5),
            (int)(render_fps + 0.5)
        );
        SDL_SetWindowTitle(window->ptr, title);
        window->title_timer = 0.0;
    }
}

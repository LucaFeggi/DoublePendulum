#include "input.h"

#include <SDL.h>

void input_poll(bool *quit) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_QUIT:
            *quit = true;
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE)
                *quit = true;
            break;
        default:
            break;
        }
    }
}
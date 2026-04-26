#include "renderer_sdl.h"

#include "../../config.h"
#include "color_utils.h"

bool renderer_sdl_init(RendererSDL *renderer_sdl, Window *window){
    renderer_sdl->win_ptr = window->ptr;
    renderer_sdl->ptr = SDL_CreateRenderer(window->ptr, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer_sdl->ptr == NULL){
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer_sdl->ptr, SDL_BLENDMODE_BLEND);

#if TRAIL
    renderer_sdl->trail = (Trail *)malloc(TOTAL_PENDULUMS * sizeof(Trail));
    for(int i = 0; i < TOTAL_PENDULUMS; i++) {
        trail_init(&renderer_sdl->trail[i]); // Initialize each trail
    }
#endif
    return true;
}

void renderer_sdl_quit(RendererSDL *renderer_sdl){

    if(renderer_sdl->ptr != NULL){
        SDL_DestroyRenderer(renderer_sdl->ptr);
    }
    renderer_sdl->ptr = NULL;
}

void renderer_sdl_render(RendererSDL *renderer_sdl, RenderData *render_data){
    int w, h;
    SDL_GetWindowSize(renderer_sdl->win_ptr, &w, &h);
    int center_x = w / 2;
    int center_y = h / 2;
    float max_rend_len = (w < h) ? w / 5.0f : h / 5.0f;  // computing render lengths dynamically
    SDL_SetRenderDrawColor(renderer_sdl->ptr, 0x0, 0x0, 0x0, 0x0);
    SDL_RenderClear(renderer_sdl->ptr);
    for(int i = 0; i < TOTAL_PENDULUMS; i++) {
        //per fare qualcosa di fatto bene dovrei calcolare da qui
        double len0 = render_data->pen_data[i].len[0] / render_data->max_len * max_rend_len;
        double len1 = render_data->pen_data[i].len[1] / render_data->max_len * max_rend_len;

        int x0 = center_x + (int)(len0 * sin(render_data->pen_data[i].angle[0]));
        int y0 = center_y + (int)(len0 * cos(render_data->pen_data[i].angle[0]));

        int x1 = x0 + (int)(len1 * sin(render_data->pen_data[i].angle[1]));
        int y1 = y0 + (int)(len1 * cos(render_data->pen_data[i].angle[1]));

        SDL_Color color[2];
        color_get_double_pendulum(&render_data->pen_data[i], render_data->max_ang_vel, color);
        // a qui in parallelo su cpu

        // e poi fare un for sequenziale per queste operazioni su gpu. STESSO RAGIONAMENTO PER LA TRAIL
        SDL_SetRenderDrawColor(renderer_sdl->ptr, color[0].r, color[0].g, color[0].b, color[0].a);
        SDL_RenderDrawLine(renderer_sdl->ptr, center_x, center_y, x0, y0);

        SDL_SetRenderDrawColor(renderer_sdl->ptr, color[1].r, color[1].g, color[1].b, color[1].a);
        SDL_RenderDrawLine(renderer_sdl->ptr, x0, y0, x1, y1);

#if TRAIL
        
        
            trail_add(&renderer_sdl->trail[i], x1, y1, color[1], w, h);
            trail_render(&renderer_sdl->trail[i], w, h, renderer_sdl->ptr);
        
#endif

    }
    SDL_RenderPresent(renderer_sdl->ptr);
}

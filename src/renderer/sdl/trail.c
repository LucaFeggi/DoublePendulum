#include "trail.h"

#include "../../config/config.h"

#include <stdlib.h>

#define ROD_LINES_PER_PENDULUM 2

bool trail_layer_init(TrailLayer *trail, int pendulum_count) {
    trail->texture = NULL;
    trail->last_tip = NULL;
    trail->has_last = NULL;
    trail->pendulum_count = pendulum_count;
    trail->w = 0;
    trail->h = 0;

    if(pendulum_count <= 0) {
        return true;
    }

    trail->last_tip = (SDL_FPoint *)calloc((size_t)pendulum_count, sizeof(SDL_FPoint));
    trail->has_last = (bool *)calloc((size_t)pendulum_count, sizeof(bool));
    if(!trail->last_tip || !trail->has_last) {
        trail_layer_quit(trail);
        return false;
    }

    return true;
}

void trail_layer_quit(TrailLayer *trail) {
    if(!trail) {
        return;
    }

    if(trail->texture) {
        SDL_DestroyTexture(trail->texture);
        trail->texture = NULL;
    }

    free(trail->last_tip);
    free(trail->has_last);
    trail->last_tip = NULL;
    trail->has_last = NULL;
    trail->pendulum_count = 0;
    trail->w = 0;
    trail->h = 0;
}

static bool trail_layer_create_texture(TrailLayer *trail, SDL_Renderer *renderer, int w, int h) {
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w,
        h
    );
    if(!texture) {
        SDL_Log("Could not create trail render target: %s", SDL_GetError());
        return false;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    if(trail->texture) {
        SDL_RenderCopy(renderer, trail->texture, NULL, NULL);
        SDL_DestroyTexture(trail->texture);
    }

    SDL_SetRenderTarget(renderer, old_target);

    trail->texture = texture;
    trail->w = w;
    trail->h = h;
    return true;
}

static bool trail_layer_ensure_size(TrailLayer *trail, SDL_Renderer *renderer, int w, int h) {
    if(w <= 0 || h <= 0) {
        return false;
    }

    if(trail->texture && trail->w == w && trail->h == h) {
        return true;
    }

    return trail_layer_create_texture(trail, renderer, w, h);
}

bool trail_layer_update(TrailLayer *trail, SDL_Renderer *renderer, const RenderLine *rod_lines, int w, int h) {
    if(!trail || !renderer || !rod_lines || trail->pendulum_count <= 0) {
        return false;
    }

    if(!trail_layer_ensure_size(trail, renderer, w, h)) {
        return false;
    }

    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, trail->texture);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, TRAIL_FADE_ALPHA);
    SDL_RenderFillRect(renderer, NULL);

    for(int i = 0; i < trail->pendulum_count; ++i) {
        const RenderLine *tip_rod = &rod_lines[i * ROD_LINES_PER_PENDULUM + 1];
        SDL_FPoint tip = { (float)tip_rod->x1, (float)tip_rod->y1 };

        if(trail->has_last[i]) {
            SDL_SetRenderDrawColor(renderer, tip_rod->color.r, tip_rod->color.g, tip_rod->color.b, tip_rod->color.a);
            SDL_RenderDrawLineF(renderer, trail->last_tip[i].x, trail->last_tip[i].y, tip.x, tip.y);
        }

        trail->last_tip[i] = tip;
        trail->has_last[i] = true;
    }

    SDL_SetRenderTarget(renderer, old_target);
    return true;
}

void trail_layer_draw(const TrailLayer *trail, SDL_Renderer *renderer) {
    if(!trail || !renderer || !trail->texture) {
        return;
    }

    SDL_RenderCopy(renderer, trail->texture, NULL, NULL);
}

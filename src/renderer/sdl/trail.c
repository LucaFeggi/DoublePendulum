#include "trail.h"

#include "../../config/config.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define ROD_LINES_PER_PENDULUM 2

bool trail_layer_init(TrailLayer *trail, int pendulum_count) {
    trail->buckets = NULL;
    trail->last_tip = NULL;
    trail->has_last = NULL;
    line_batch_init(&trail->line_batch, 0);
    trail->bucket_timer = 0.0f;
    trail->current_bucket = 0;
    trail->bucket_count = 0;
    trail->pendulum_count = pendulum_count;
    trail->w = 0;
    trail->h = 0;

    if(pendulum_count <= 0) {
        return true;
    }

    trail->bucket_count = TRAIL_BUCKET_COUNT;
    trail->buckets = (TrailBucket *)calloc((size_t)trail->bucket_count, sizeof(TrailBucket));
    trail->last_tip = (SDL_FPoint *)calloc((size_t)pendulum_count, sizeof(SDL_FPoint));
    trail->has_last = (bool *)calloc((size_t)pendulum_count, sizeof(bool));
    if(!trail->buckets || !trail->last_tip || !trail->has_last || !line_batch_init(&trail->line_batch, pendulum_count)) {
        trail_layer_quit(trail);
        return false;
    }

    return true;
}

void trail_layer_quit(TrailLayer *trail) {
    if(!trail) {
        return;
    }

    if(trail->buckets) {
        for(int i = 0; i < trail->bucket_count; ++i) {
            if(trail->buckets[i].texture) {
                SDL_DestroyTexture(trail->buckets[i].texture);
                trail->buckets[i].texture = NULL;
            }
        }
    }

    free(trail->buckets);
    free(trail->last_tip);
    free(trail->has_last);
    line_batch_quit(&trail->line_batch);
    trail->buckets = NULL;
    trail->last_tip = NULL;
    trail->has_last = NULL;
    trail->bucket_timer = 0.0f;
    trail->current_bucket = 0;
    trail->bucket_count = 0;
    trail->pendulum_count = 0;
    trail->w = 0;
    trail->h = 0;
}

static SDL_Texture *trail_layer_create_texture(SDL_Renderer *renderer, int w, int h) {
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w,
        h
    );
    if(!texture) {
        SDL_Log("Could not create trail render target: %s", SDL_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}

static bool trail_layer_clear_target(SDL_Renderer *renderer, SDL_Texture *texture) {
    if(SDL_SetRenderTarget(renderer, texture) != 0) {
        SDL_Log("Could not set trail render target: %s", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    return true;
}

static bool trail_layer_copy_texture(SDL_Renderer *renderer, SDL_Texture *src) {
    SDL_BlendMode old_blend_mode = SDL_BLENDMODE_BLEND;
    Uint8 old_alpha = 255;

    SDL_GetTextureBlendMode(src, &old_blend_mode);
    SDL_GetTextureAlphaMod(src, &old_alpha);
    SDL_SetTextureBlendMode(src, SDL_BLENDMODE_NONE);
    SDL_SetTextureAlphaMod(src, 255);

    bool success = SDL_RenderCopy(renderer, src, NULL, NULL) == 0;
    if(!success) {
        SDL_Log("Could not copy old trail render target: %s", SDL_GetError());
    }

    SDL_SetTextureAlphaMod(src, old_alpha);
    SDL_SetTextureBlendMode(src, old_blend_mode);
    return success;
}

static void trail_layer_destroy_textures(SDL_Texture **textures, int texture_count) {
    if(!textures) {
        return;
    }

    for(int i = 0; i < texture_count; ++i) {
        if(textures[i]) {
            SDL_DestroyTexture(textures[i]);
        }
    }
}

static bool trail_layer_recreate_textures(TrailLayer *trail, SDL_Renderer *renderer, int w, int h) {
    SDL_Texture **new_textures = (SDL_Texture **)calloc((size_t)trail->bucket_count, sizeof(SDL_Texture *));
    if(!new_textures) {
        return false;
    }

    for(int i = 0; i < trail->bucket_count; ++i) {
        new_textures[i] = trail_layer_create_texture(renderer, w, h);
        if(!new_textures[i]) {
            trail_layer_destroy_textures(new_textures, trail->bucket_count);
            free(new_textures);
            return false;
        }
    }

    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);
    bool success = true;

    for(int i = 0; i < trail->bucket_count; ++i) {
        success = trail_layer_clear_target(renderer, new_textures[i]);
        if(success && trail->buckets[i].texture) {
            success = trail_layer_copy_texture(renderer, trail->buckets[i].texture);
        }

        if(!success) {
            break;
        }
    }

    SDL_SetRenderTarget(renderer, old_target);

    if(!success) {
        trail_layer_destroy_textures(new_textures, trail->bucket_count);
        free(new_textures);
        return false;
    }

    for(int i = 0; i < trail->bucket_count; ++i) {
        if(trail->buckets[i].texture) {
            SDL_DestroyTexture(trail->buckets[i].texture);
        }
        trail->buckets[i].texture = new_textures[i];
    }

    free(new_textures);
    trail->w = w;
    trail->h = h;
    return true;
}

static bool trail_layer_ensure_size(TrailLayer *trail, SDL_Renderer *renderer, int w, int h) {
    if(w <= 0 || h <= 0 || trail->bucket_count <= 0) {
        return false;
    }

    if(trail->buckets[0].texture && trail->w == w && trail->h == h) {
        return true;
    }

    return trail_layer_recreate_textures(trail, renderer, w, h);
}

static bool trail_layer_clear_bucket(TrailLayer *trail, SDL_Renderer *renderer, int bucket_index) {
    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);
    bool success = trail_layer_clear_target(renderer, trail->buckets[bucket_index].texture);
    SDL_SetRenderTarget(renderer, old_target);
    return success;
}

static bool trail_layer_clear_all_buckets(TrailLayer *trail, SDL_Renderer *renderer) {
    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);

    for(int i = 0; i < trail->bucket_count; ++i) {
        if(!trail_layer_clear_target(renderer, trail->buckets[i].texture)) {
            SDL_SetRenderTarget(renderer, old_target);
            return false;
        }
    }

    SDL_SetRenderTarget(renderer, old_target);
    return true;
}

static bool trail_layer_advance_time(TrailLayer *trail, SDL_Renderer *renderer, float delta_time) {
    if(delta_time <= 0.0f) {
        return true;
    }

    if(delta_time >= TRAIL_DURATION_SECONDS) {
        trail->bucket_timer = 0.0f;
        trail->current_bucket = 0;
        memset(trail->has_last, 0, (size_t)trail->pendulum_count * sizeof(bool));
        return trail_layer_clear_all_buckets(trail, renderer);
    }

    trail->bucket_timer += delta_time;

    while(trail->bucket_timer >= TRAIL_BUCKET_SECONDS) {
        trail->bucket_timer -= TRAIL_BUCKET_SECONDS;
        trail->current_bucket = (trail->current_bucket + 1) % trail->bucket_count;

        if(!trail_layer_clear_bucket(trail, renderer, trail->current_bucket)) {
            return false;
        }
    }

    return true;
}

static Uint8 trail_layer_alpha_for_age(float age_seconds) {
    if(age_seconds >= TRAIL_DURATION_SECONDS) {
        return 0;
    }

    float t = 1.0f - (age_seconds / TRAIL_DURATION_SECONDS);
    float shaped_alpha = powf(t, TRAIL_FADE_GAMMA);
    int alpha = (int)(shaped_alpha * 255.0f + 0.5f);

    if(alpha < 0) {
        return 0;
    }
    if(alpha > 255) {
        return 255;
    }
    return (Uint8)alpha;
}

bool trail_layer_update(TrailLayer *trail, SDL_Renderer *renderer, const RenderLine *rod_lines, int w, int h, float delta_time) {
    if(!trail || !renderer || !rod_lines || trail->pendulum_count <= 0) {
        return false;
    }

    if(!trail_layer_ensure_size(trail, renderer, w, h)) {
        return false;
    }

    if(!trail_layer_advance_time(trail, renderer, delta_time)) {
        return false;
    }

    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);
    SDL_Texture *current_texture = trail->buckets[trail->current_bucket].texture;

    if(SDL_SetRenderTarget(renderer, current_texture) != 0) {
        SDL_Log("Could not set current trail bucket: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, old_target);
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    line_batch_reset(&trail->line_batch);

    for(int i = 0; i < trail->pendulum_count; ++i) {
        const RenderLine *tip_rod = &rod_lines[i * ROD_LINES_PER_PENDULUM + 1];
        SDL_FPoint tip = { (float)tip_rod->x1, (float)tip_rod->y1 };

        if(trail->has_last[i]) {
            if(!line_batch_add(
                &trail->line_batch,
                trail->last_tip[i].x,
                trail->last_tip[i].y,
                tip.x,
                tip.y,
                TRAIL_WIDTH_PIXELS,
                tip_rod->color
            )) {
                SDL_Log("Could not add trail line to SDL geometry batch.");
                SDL_SetRenderTarget(renderer, old_target);
                return false;
            }
        }

        trail->last_tip[i] = tip;
        trail->has_last[i] = true;
    }

    if(!line_batch_draw(&trail->line_batch, renderer)) {
        SDL_Log("Could not draw SDL trail geometry batch: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, old_target);
        return false;
    }

    SDL_SetRenderTarget(renderer, old_target);
    return true;
}

void trail_layer_draw(const TrailLayer *trail, SDL_Renderer *renderer) {
    if(!trail || !renderer || !trail->buckets || trail->bucket_count <= 0) {
        return;
    }

    for(int offset = trail->bucket_count - 1; offset >= 0; --offset) {
        int bucket_index = trail->current_bucket - offset;
        if(bucket_index < 0) {
            bucket_index += trail->bucket_count;
        }

        SDL_Texture *texture = trail->buckets[bucket_index].texture;
        if(!texture) {
            continue;
        }

        float age_seconds = (float)offset * TRAIL_BUCKET_SECONDS + trail->bucket_timer;
        Uint8 alpha = trail_layer_alpha_for_age(age_seconds);
        if(alpha == 0) {
            continue;
        }

        SDL_SetTextureAlphaMod(texture, alpha);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }
}

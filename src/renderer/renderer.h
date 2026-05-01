#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include "../app/render_data.h"
#include "../app/window.h"
#include "../utils/threadpool.h"

#include "renderer_backend.h"

bool renderer_init(Renderer *renderer, Window *window);
void renderer_quit(Renderer *renderer);
void renderer_render(Renderer *renderer, RenderData *render_data, ThreadPool *threadpool, int w, int h);

#endif // RENDERER_RENDERER_H

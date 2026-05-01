#include "line_batch.h"

#include <math.h>
#include <stdlib.h>

#define LINE_BATCH_VERTICES_PER_LINE 4
#define LINE_BATCH_INDICES_PER_LINE 6

bool line_batch_init(LineBatch *batch, int line_capacity) {
    if(!batch) {
        return false;
    }

    batch->vertices = NULL;
    batch->indices = NULL;
    batch->vertex_count = 0;
    batch->index_count = 0;
    batch->line_capacity = 0;

    if(line_capacity <= 0) {
        return true;
    }

    batch->vertices = (SDL_Vertex *)malloc((size_t)line_capacity * LINE_BATCH_VERTICES_PER_LINE * sizeof(SDL_Vertex));
    batch->indices = (int *)malloc((size_t)line_capacity * LINE_BATCH_INDICES_PER_LINE * sizeof(int));
    if(!batch->vertices || !batch->indices) {
        line_batch_quit(batch);
        return false;
    }

    batch->line_capacity = line_capacity;
    return true;
}

void line_batch_quit(LineBatch *batch) {
    if(!batch) {
        return;
    }

    free(batch->vertices);
    free(batch->indices);
    batch->vertices = NULL;
    batch->indices = NULL;
    batch->vertex_count = 0;
    batch->index_count = 0;
    batch->line_capacity = 0;
}

void line_batch_reset(LineBatch *batch) {
    if(!batch) {
        return;
    }

    batch->vertex_count = 0;
    batch->index_count = 0;
}

bool line_batch_add(LineBatch *batch, float x0, float y0, float x1, float y1, float width, SDL_Color color) {
    if(!batch || !batch->vertices || !batch->indices || width <= 0.0f) {
        return false;
    }

    if(batch->vertex_count + LINE_BATCH_VERTICES_PER_LINE > batch->line_capacity * LINE_BATCH_VERTICES_PER_LINE
        || batch->index_count + LINE_BATCH_INDICES_PER_LINE > batch->line_capacity * LINE_BATCH_INDICES_PER_LINE) {
        return false;
    }

    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    if(len <= 0.0001f) {
        return true;
    }

    float ux = dx / len;
    float uy = dy / len;
    float half_width = width * 0.5f;
    float nx = -uy * half_width;
    float ny = ux * half_width;

    // Square caps cover endpoints and avoid tiny gaps between consecutive trail segments.
    x0 -= ux * half_width;
    y0 -= uy * half_width;
    x1 += ux * half_width;
    y1 += uy * half_width;

    int v = batch->vertex_count;
    int i = batch->index_count;

    batch->vertices[v + 0] = (SDL_Vertex){ { x0 + nx, y0 + ny }, color, { 0.0f, 0.0f } };
    batch->vertices[v + 1] = (SDL_Vertex){ { x0 - nx, y0 - ny }, color, { 0.0f, 0.0f } };
    batch->vertices[v + 2] = (SDL_Vertex){ { x1 - nx, y1 - ny }, color, { 0.0f, 0.0f } };
    batch->vertices[v + 3] = (SDL_Vertex){ { x1 + nx, y1 + ny }, color, { 0.0f, 0.0f } };

    batch->indices[i + 0] = v + 0;
    batch->indices[i + 1] = v + 1;
    batch->indices[i + 2] = v + 2;
    batch->indices[i + 3] = v + 0;
    batch->indices[i + 4] = v + 2;
    batch->indices[i + 5] = v + 3;

    batch->vertex_count += LINE_BATCH_VERTICES_PER_LINE;
    batch->index_count += LINE_BATCH_INDICES_PER_LINE;
    return true;
}

bool line_batch_draw(const LineBatch *batch, SDL_Renderer *renderer) {
    if(!batch || !renderer || batch->vertex_count <= 0 || batch->index_count <= 0) {
        return true;
    }

    return SDL_RenderGeometry(
        renderer,
        NULL,
        batch->vertices,
        batch->vertex_count,
        batch->indices,
        batch->index_count
    ) == 0;
}

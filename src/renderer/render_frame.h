#ifndef RENDERER_RENDER_FRAME_H
#define RENDERER_RENDER_FRAME_H

#include "../simulation/simulation.h"
#include "../simulation/simulation_snapshot.h"

typedef struct {
    float len[2];
    float max_len;
    float max_ang_vel;
    PendulumRenderSample *pen_data;
} RenderFrame;

bool render_frame_init(RenderFrame *render_frame, const Simulation *simulation);
void render_frame_quit(RenderFrame *render_frame);
void render_frame_pack(RenderFrame *render_frame, const Simulation *simulation, float current_max_ang_vel, float delta_time);

#endif // RENDERER_RENDER_FRAME_H

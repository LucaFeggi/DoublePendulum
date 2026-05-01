#ifndef APP_RENDER_DATA_H
#define APP_RENDER_DATA_H

#include "../simulation/simulation.h"
#include "../simulation/simulation_snapshot.h"

typedef PendulumRenderSample PendulumRenderData;

typedef struct {
    float max_len;
    float max_ang_vel;
    PendulumRenderData *pen_data;
} RenderData;

bool render_data_init(RenderData *render_data, const Simulation *simulation);
void render_data_quit(RenderData *render_data);
void render_data_pack(RenderData *render_data, const Simulation *simulation, float delta_time);

#endif // APP_RENDER_DATA_H

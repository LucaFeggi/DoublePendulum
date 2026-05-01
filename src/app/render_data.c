#include "render_data.h"

#include "../config/config.h"

#include <stdlib.h>

bool render_data_init(RenderData *rd, const Simulation *sim) {
    rd->max_len = (float)simulation_get_max_len(sim);
    rd->max_ang_vel = 0.0f;
    rd->pen_data = (PendulumRenderData *)malloc((size_t)TOTAL_PENDULUMS * sizeof(PendulumRenderData));
    if(!rd->pen_data) {
        return false;
    }

    simulation_fill_render_samples(sim, rd->pen_data, TOTAL_PENDULUMS);
    return true;
}

void render_data_quit(RenderData *rd) {
    if(!rd) {
        return;
    }

    free(rd->pen_data);
    rd->pen_data = NULL;
    rd->max_len = 0.0f;
    rd->max_ang_vel = 0.0f;
}

void render_data_pack(RenderData *rd, const Simulation *sim) {
    float current_max_ang_vel = (float)simulation_get_max_ang_vel(sim);
    float decayed_max_ang_vel = rd->max_ang_vel * (float)COLOR_DECAY;
    rd->max_ang_vel = current_max_ang_vel > decayed_max_ang_vel
        ? current_max_ang_vel
        : decayed_max_ang_vel;

    simulation_fill_render_samples(sim, rd->pen_data, TOTAL_PENDULUMS);
}

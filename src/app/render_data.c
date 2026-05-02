#include "render_data.h"

#include "../config/config.h"

#include <math.h>
#include <stdlib.h>

bool render_data_init(RenderData *rd, const Simulation *sim) {
    rd->len[0] = (float)simulation_get_len(sim, 0);
    rd->len[1] = (float)simulation_get_len(sim, 1);
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
    rd->len[0] = 0.0f;
    rd->len[1] = 0.0f;
    rd->max_len = 0.0f;
    rd->max_ang_vel = 0.0f;
}

static float render_data_color_decay_for_delta_time(float delta_time) {
    if(!(delta_time > 0.0f)) {
        return 1.0f;
    }

    return powf((float)COLOR_DECAY, delta_time * (float)COLOR_DECAY_REFERENCE_FPS);
}

void render_data_pack(RenderData *rd, const Simulation *sim, float delta_time) {
    float current_max_ang_vel = (float)simulation_get_max_ang_vel(sim);
    float decayed_max_ang_vel = rd->max_ang_vel * render_data_color_decay_for_delta_time(delta_time);
    rd->max_ang_vel = current_max_ang_vel > decayed_max_ang_vel
        ? current_max_ang_vel
        : decayed_max_ang_vel;

    simulation_fill_render_samples(sim, rd->pen_data, TOTAL_PENDULUMS);
}

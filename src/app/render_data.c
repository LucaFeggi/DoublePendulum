#include "render_data.h"

#include "../config.h"

#include <stdlib.h>

bool render_data_init(RenderData *rd, const Simulation *sim) {
    rd->max_len = (float)sim->max_len;
    rd->max_ang_vel = 0.0f;
    rd->pen_data = malloc(TOTAL_PENDULUMS * sizeof(PendulumRenderData));
    if(!rd->pen_data){
        return false;
    }
    for(int i = 0; i < TOTAL_PENDULUMS; ++i) {
        rd->pen_data[i].len[0] = (float)sim->pendulum[i].rod[0].len;
        rd->pen_data[i].len[1] = (float)sim->pendulum[i].rod[1].len;
    }
    return true;
}


void render_data_quit(RenderData *sim_data){
    free(sim_data->pen_data);
}

void render_data_pack(RenderData *rd, const Simulation *sim) {
    float current_max_ang_vel = (float)sim->max_ang_vel;
    float decayed_max_ang_vel = rd->max_ang_vel * (float)COLOR_DECAY;
    rd->max_ang_vel = current_max_ang_vel > decayed_max_ang_vel
        ? current_max_ang_vel
        : decayed_max_ang_vel;

    for(int i = 0; i < TOTAL_PENDULUMS; ++i) {
        rd->pen_data[i].angle[0] = (float)sim->pendulum[i].rod[0].angle;
        rd->pen_data[i].ang_vel[0] = (float)sim->pendulum[i].rod[0].ang_vel;
        rd->pen_data[i].angle[1] = (float)sim->pendulum[i].rod[1].angle;
        rd->pen_data[i].ang_vel[1] = (float)sim->pendulum[i].rod[1].ang_vel;
    }
}

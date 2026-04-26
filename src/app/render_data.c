#include "render_data.h"

#include "../config.h"

#include <stdlib.h>

bool render_data_init(RenderData *sim_data, const double max_len){
    sim_data->max_len = max_len;   // never changes
    sim_data->max_ang_vel = 0.0f;   // always changes
    sim_data->pen_data = (PendulumRenderData *)malloc(TOTAL_PENDULUMS * sizeof(PendulumRenderData));    // always changes
    if(sim_data->pen_data == NULL){
        return false;
    }
    return true;
}

void render_data_quit(RenderData *sim_data){
    free(sim_data->pen_data);
}

void render_data_pack(RenderData *sim_data, const Simulation *simulation){
    sim_data->max_ang_vel = (float)simulation->max_ang_vel;
    for(int i = 0; i < TOTAL_PENDULUMS; i++){
        sim_data->pen_data[i].len[0] = (float)simulation->pendulum[i].rod[0].len;
        sim_data->pen_data[i].angle[0] = (float)simulation->pendulum[i].rod[0].angle;
        sim_data->pen_data[i].ang_vel[0] = (float)simulation->pendulum[i].rod[0].ang_vel;
        sim_data->pen_data[i].len[1] = (float)simulation->pendulum[i].rod[1].len;
        sim_data->pen_data[i].angle[1] = (float)simulation->pendulum[i].rod[1].angle;
        sim_data->pen_data[i].ang_vel[1] = (float)simulation->pendulum[i].rod[1].ang_vel;
    }
}
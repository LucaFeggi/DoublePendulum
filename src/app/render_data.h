#ifndef APP_RENDER_DATA_H
#define APP_RENDER_DATA_H

#include "../simulation/simulation.h"

typedef struct{
    float len[2];       // rendering
    float angle[2];     // rendering
    float ang_vel[2];   // coloring
}PendulumRenderData;

typedef struct{
    float max_len;      // max length of a rod - rendering
    float max_ang_vel;  // coloring
    PendulumRenderData *pen_data;
}RenderData;    // all data from simulation for the render on GPU

bool render_data_init(RenderData *sim_data, const double max_len);
void render_data_quit(RenderData *sim_data);
void render_data_pack(RenderData *sim_data, const Simulation *simulation);

#endif // APP_RENDER_DATA_H

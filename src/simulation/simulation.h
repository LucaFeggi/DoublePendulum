#ifndef SIMULATION_SIMULATION_H
#define SIMULATION_SIMULATION_H

#include "pendulum.h"
#include "simulation_snapshot.h"

#include <stdbool.h>

typedef struct {
    PendulumParams params;
    PendulumState *state;
    double max_len;
    double max_ang_vel;
} Simulation;

bool simulation_init_custom(Simulation *simulation);
bool simulation_init_default(Simulation *simulation);
double simulation_update_range(Simulation *simulation, int start_index, int end_index, int steps);
void simulation_update_steps(Simulation *simulation, int steps);
void simulation_fill_render_samples(const Simulation *simulation, PendulumRenderSample *out, int count);
void simulation_set_max_ang_vel(Simulation *simulation, double max_ang_vel);
int simulation_get_count(const Simulation *simulation);
double simulation_get_max_len(const Simulation *simulation);
double simulation_get_max_ang_vel(const Simulation *simulation);
void simulation_quit(Simulation *simulation);

#endif // SIMULATION_SIMULATION_H

#ifndef SIMULATION_SIMULATION_H
#define SIMULATION_SIMULATION_H

#include "pendulum.h"
#include "simulation_snapshot.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    PendulumParams params;
    PendulumState *state;
} Simulation;

bool simulation_init_custom(Simulation *simulation);
bool simulation_init_default(Simulation *simulation);
double simulation_update_range(Simulation *simulation, size_t start_index, size_t end_index, int steps);
double simulation_update_steps(Simulation *simulation, int steps);
void simulation_fill_render_samples(const Simulation *simulation, PendulumRenderSample *out, size_t count);
size_t simulation_get_count(const Simulation *simulation);
double simulation_get_len(const Simulation *simulation, int rod_index);
void simulation_quit(Simulation *simulation);

#endif // SIMULATION_SIMULATION_H

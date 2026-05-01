#ifndef SIMULATION_SIMULATION_H
#define SIMULATION_SIMULATION_H

#include "pendulum.h"
#include "simulation_snapshot.h"
#include "../utils/threadpool.h"

#include <stdbool.h>

typedef struct {
    PendulumParams params;
    PendulumState *state;
    double max_len;
    double max_ang_vel;
    double *thread_max_ang_vel;
    int thread_max_capacity;
} Simulation;

bool simulation_init_custom(Simulation *simulation, int worker_threads);
bool simulation_init_default(Simulation *simulation, int worker_threads);
void simulation_update_steps(Simulation *simulation, ThreadPool *threadpool, int steps);
void simulation_fill_render_samples(const Simulation *simulation, PendulumRenderSample *out, int count);
double simulation_get_max_len(const Simulation *simulation);
double simulation_get_max_ang_vel(const Simulation *simulation);
void simulation_quit(Simulation *simulation);

#endif // SIMULATION_SIMULATION_H

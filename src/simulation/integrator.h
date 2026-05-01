#ifndef SIMULATION_INTEGRATOR_H
#define SIMULATION_INTEGRATOR_H

#include "pendulum.h"

void integrator_pendulum_update(PendulumState *state, const PendulumParams *params);

#endif // SIMULATION_INTEGRATOR_H

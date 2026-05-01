#ifndef SIMULATION_PENDULUM_EQUATIONS_H
#define SIMULATION_PENDULUM_EQUATIONS_H

#include "pendulum.h"

typedef struct {
    double d_angle[2];
    double d_ang_vel[2];
} PendulumDerivative;

void pendulum_compute_acceleration(const PendulumParams *params, const PendulumState *state, double ang_acc[2]);
PendulumDerivative pendulum_compute_derivative(const PendulumParams *params, const PendulumState *state);

#endif // SIMULATION_PENDULUM_EQUATIONS_H

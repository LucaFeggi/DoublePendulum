#ifndef SIMULATION_PENDULUM_EQUATIONS_H
#define SIMULATION_PENDULUM_EQUATIONS_H

#include "pendulum.h"

typedef struct{
    double angle[2];
    double ang_vel[2];
}PendulumState;

typedef struct{
    double d_angle[2];
    double d_ang_vel[2];
}PendulumDerivative;

PendulumState pendulum_get_state(const Pendulum *pendulum);
void pendulum_set_state(Pendulum *pendulum, const PendulumState *state);
void pendulum_compute_acceleration(const Pendulum *pendulum, const PendulumState *state, double ang_acc[2]);
PendulumDerivative pendulum_compute_derivative(const Pendulum *pendulum, const PendulumState *state);

#endif // SIMULATION_PENDULUM_EQUATIONS_H

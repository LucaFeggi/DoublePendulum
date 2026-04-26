#include "pendulum_equations.h"

#include "../config.h"

#include <math.h>

PendulumState pendulum_get_state(const Pendulum *pendulum) {
    PendulumState state;
    state.angle[0] = pendulum->rod[0].angle;
    state.angle[1] = pendulum->rod[1].angle;
    state.ang_vel[0] = pendulum->rod[0].ang_vel;
    state.ang_vel[1] = pendulum->rod[1].ang_vel;
    return state;
}

void pendulum_set_state(Pendulum *pendulum, const PendulumState *state) {
    pendulum->rod[0].angle = state->angle[0];
    pendulum->rod[1].angle = state->angle[1];
    pendulum->rod[0].ang_vel = state->ang_vel[0];
    pendulum->rod[1].ang_vel = state->ang_vel[1];
}

void pendulum_compute_acceleration(const Pendulum *pendulum, const PendulumState *state, double ang_acc[2]) {
    double m0 = pendulum->rod[0].mass;
    double m1 = pendulum->rod[1].mass;
    double l0 = pendulum->rod[0].len;
    double l1 = pendulum->rod[1].len;

    double theta0 = state->angle[0];
    double theta1 = state->angle[1];
    double omega0 = state->ang_vel[0];
    double omega1 = state->ang_vel[1];

    double delta = theta0 - theta1;
    double denominator = 2 * m0 + m1 - m1 * cos(2 * delta);

    double numerator0 = -G * (2 * m0 + m1) * sin(theta0)
        - m1 * G * sin(theta0 - 2 * theta1)
        - 2 * sin(delta) * m1 * (omega1 * omega1 * l1 + omega0 * omega0 * l0 * cos(delta));

    double numerator1 = 2 * sin(delta)
        * (omega0 * omega0 * l0 * (m0 + m1)
        + G * (m0 + m1) * cos(theta0)
        + omega1 * omega1 * l1 * m1 * cos(delta));

    ang_acc[0] = numerator0 / (l0 * denominator);
    ang_acc[1] = numerator1 / (l1 * denominator);
}

PendulumDerivative pendulum_compute_derivative(const Pendulum *pendulum, const PendulumState *state) {
    PendulumDerivative derivative;
    derivative.d_angle[0] = state->ang_vel[0];
    derivative.d_angle[1] = state->ang_vel[1];
    pendulum_compute_acceleration(pendulum, state, derivative.d_ang_vel);
    return derivative;
}

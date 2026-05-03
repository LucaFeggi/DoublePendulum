#include "pendulum_equations.h"

#include "../config/simulation_config.h"

#include <math.h>

void pendulum_compute_acceleration(const PendulumParams *params, const PendulumState *state, double ang_acc[2]) {
    double m0 = params->mass[0];
    double m1 = params->mass[1];
    double l0 = params->len[0];
    double l1 = params->len[1];

    double theta0 = state->angle[0];
    double theta1 = state->angle[1];
    double omega0 = state->ang_vel[0];
    double omega1 = state->ang_vel[1];

    double s0 = sin(theta0);
    double c0 = cos(theta0);
    double s1 = sin(theta1);
    double c1 = cos(theta1);

    double sin_delta = s0 * c1 - c0 * s1;
    double cos_delta = c0 * c1 + s0 * s1;
    double cos_2_delta = 2.0 * cos_delta * cos_delta - 1.0;

    double sin_2_theta1 = 2.0 * s1 * c1;
    double cos_2_theta1 = c1 * c1 - s1 * s1;
    double sin_theta0_minus_2_theta1 = s0 * cos_2_theta1 - c0 * sin_2_theta1;

    double omega0_sq = omega0 * omega0;
    double omega1_sq = omega1 * omega1;
    double mass_sum = m0 + m1;
    double two_m0_plus_m1 = 2.0 * m0 + m1;

    double denominator = two_m0_plus_m1 - m1 * cos_2_delta;

    double numerator0 = -G * two_m0_plus_m1 * s0
        - m1 * G * sin_theta0_minus_2_theta1
        - 2.0 * sin_delta * m1 * (omega1_sq * l1 + omega0_sq * l0 * cos_delta);

    double numerator1 = 2.0 * sin_delta
        * (omega0_sq * l0 * mass_sum
        + G * mass_sum * c0
        + omega1_sq * l1 * m1 * cos_delta);

    ang_acc[0] = numerator0 / (l0 * denominator);
    ang_acc[1] = numerator1 / (l1 * denominator);
}

PendulumDerivative pendulum_compute_derivative(const PendulumParams *params, const PendulumState *state) {
    PendulumDerivative derivative;
    derivative.d_angle[0] = state->ang_vel[0];
    derivative.d_angle[1] = state->ang_vel[1];
    pendulum_compute_acceleration(params, state, derivative.d_ang_vel);
    return derivative;
}

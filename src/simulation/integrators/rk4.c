#include "../integrator.h"

#include "../../config/simulation_config.h"
#include "../pendulum_equations.h"

static PendulumDerivative rk4_evaluate(
    const PendulumParams *params,
    PendulumState state,
    double dt,
    PendulumDerivative derivative
) {
    PendulumState next_state;
    next_state.angle[0] = state.angle[0] + derivative.d_angle[0] * dt;
    next_state.angle[1] = state.angle[1] + derivative.d_angle[1] * dt;
    next_state.ang_vel[0] = state.ang_vel[0] + derivative.d_ang_vel[0] * dt;
    next_state.ang_vel[1] = state.ang_vel[1] + derivative.d_ang_vel[1] * dt;

    return pendulum_compute_derivative(params, &next_state);
}

void integrator_pendulum_update(PendulumState *state, const PendulumParams *params) {
    double dt = SIMULATION_DT;
    double sixth_dt = dt / 6.0;
    PendulumState initial_state = *state;

    PendulumDerivative k1 = rk4_evaluate(params, initial_state, 0.0, (PendulumDerivative){ 0 });
    PendulumDerivative k2 = rk4_evaluate(params, initial_state, dt * 0.5, k1);
    PendulumDerivative k3 = rk4_evaluate(params, initial_state, dt * 0.5, k2);
    PendulumDerivative k4 = rk4_evaluate(params, initial_state, dt, k3);

    state->angle[0] += sixth_dt * (k1.d_angle[0] + 2.0 * k2.d_angle[0] + 2.0 * k3.d_angle[0] + k4.d_angle[0]);
    state->angle[1] += sixth_dt * (k1.d_angle[1] + 2.0 * k2.d_angle[1] + 2.0 * k3.d_angle[1] + k4.d_angle[1]);
    state->ang_vel[0] += sixth_dt * (k1.d_ang_vel[0] + 2.0 * k2.d_ang_vel[0] + 2.0 * k3.d_ang_vel[0] + k4.d_ang_vel[0]);
    state->ang_vel[1] += sixth_dt * (k1.d_ang_vel[1] + 2.0 * k2.d_ang_vel[1] + 2.0 * k3.d_ang_vel[1] + k4.d_ang_vel[1]);
}

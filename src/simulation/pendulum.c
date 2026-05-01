#include "pendulum.h"

#include "integrator.h"

void pendulum_state_init(
    PendulumState *state,
    double angle0,
    double ang_vel0,
    double angle1,
    double ang_vel1
) {
    state->angle[0] = angle0;
    state->angle[1] = angle1;
    state->ang_vel[0] = ang_vel0;
    state->ang_vel[1] = ang_vel1;
}

void pendulum_update(PendulumState *state, const PendulumParams *params) {
    integrator_pendulum_update(state, params);
}

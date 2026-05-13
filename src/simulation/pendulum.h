#ifndef SIMULATION_PENDULUM_H
#define SIMULATION_PENDULUM_H

typedef struct {
    double len[2];
    double mass[2];
} PendulumParams;

typedef struct {
    double angle[2];
    double ang_vel[2];
} PendulumState;

void pendulum_state_init(PendulumState *state, double angle0, double ang_vel0, double angle1, double ang_vel1);

void pendulum_update(PendulumState *state, const PendulumParams *params);

#endif // SIMULATION_PENDULUM_H

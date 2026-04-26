#ifndef SIMULATION_PENDULUM_H
#define SIMULATION_PENDULUM_H

#include "rod.h"

typedef struct{
    Rod rod[2];
}Pendulum;

void pendulum_init_custom(
    Pendulum *pendulum,
    double angle0, double ang_vel0, double ang_acc0, double len0, double mass0,
    double angle1, double ang_vel1, double ang_acc1, double len1, double mass1
);
void pendulum_init_default(Pendulum *pendulum, double angle);
void pendulum_update(Pendulum *pendulum);

#endif // SIMULATION_PENDULUM_H

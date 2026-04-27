#include "pendulum.h"

#include "integrator.h"

void pendulum_init_custom(
    Pendulum *pendulum,
    double angle0, double ang_vel0, double len0, double mass0,
    double angle1, double ang_vel1, double len1, double mass1
){
    rod_init_custom(&pendulum->rod[0], angle0, ang_vel0, len0, mass0);
    rod_init_custom(&pendulum->rod[1], angle1, ang_vel1, len1, mass1);
}

void pendulum_init_default(Pendulum *pendulum, double angle){
    rod_init_default(&pendulum->rod[0], angle);
    rod_init_default(&pendulum->rod[1], angle);
}

void pendulum_update(Pendulum *pendulum){
    integrator_pendulum_update(pendulum);
}

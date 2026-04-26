#include "pendulum.h"

#include "rk4.h"

void pendulum_init_custom(
    Pendulum *pendulum,
    double angle0, double ang_vel0, double ang_acc0, double len0, double mass0,
    double angle1, double ang_vel1, double ang_acc1, double len1, double mass1
){
    rod_init_custom(&pendulum->rod[0], angle0, ang_vel0, ang_acc0, len0, mass0);
    rod_init_custom(&pendulum->rod[1], angle1, ang_vel1, ang_acc1, len1, mass1);
}

void pendulum_init_default(Pendulum *pendulum, double angle){
    rod_init_default(&pendulum->rod[0], angle);
    rod_init_default(&pendulum->rod[1], angle);
}

void pendulum_update(Pendulum *pendulum){
    rk4_pendulum_update(pendulum);
}

// definire qui le funzioni per updateare il pendulum, e poi da math/rk4.h chiamare rk4_update(pendulum_formulas)
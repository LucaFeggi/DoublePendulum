#include "rod.h"

#include "../config.h"

void rod_init_custom(Rod *rod, double angle, double ang_vel, double len, double mass){
    rod->angle = angle;
    rod->ang_vel = ang_vel;
    rod->len = len;
    rod->mass = mass;
}

void rod_init_default(Rod *rod, double angle){
    rod_init_custom(rod, angle, DEFAULT_ANG_VEL, DEFAULT_LEN, DEFAULT_MASS);
}

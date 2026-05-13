#include "simulation.h"

#include "../config/simulation_config.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
    PendulumParams params;
    double angle[2];
    double angle_adder[2];
    double ang_vel[2];
} SimulationInitSpec;

static bool simulation_init_from_spec(Simulation *simulation, const SimulationInitSpec *spec) {
    simulation->params = spec->params;
    simulation->state = NULL;

    const size_t pendulum_count = (size_t)TOTAL_PENDULUMS;
    simulation->state = (PendulumState *)malloc(pendulum_count * sizeof(PendulumState));
    if(simulation->state == NULL) {
        return false;
    }

    for(size_t i = 0; i < pendulum_count; ++i) {
        pendulum_state_init(&simulation->state[i], spec->angle[0] + spec->angle_adder[0] * (double)i, spec->ang_vel[0],
                            spec->angle[1] + spec->angle_adder[1] * (double)i, spec->ang_vel[1]);
    }

    return true;
}

bool simulation_init_custom(Simulation *simulation) {
    SimulationInitSpec spec = { .params = { .len = { CUSTOM_LEN_ROD1, CUSTOM_LEN_ROD2 },
                                            .mass = { CUSTOM_MASS_ROD1, CUSTOM_MASS_ROD2 } },
                                .angle = { CUSTOM_ANGLE_ROD1, CUSTOM_ANGLE_ROD2 },
                                .angle_adder = { CUSTOM_ANGLE_ADDER_ROD1, CUSTOM_ANGLE_ADDER_ROD2 },
                                .ang_vel = { CUSTOM_ANG_VEL_ROD1, CUSTOM_ANG_VEL_ROD2 } };

    return simulation_init_from_spec(simulation, &spec);
}

bool simulation_init_default(Simulation *simulation) {
    SimulationInitSpec spec = { .params = { .len = { DEFAULT_LEN, DEFAULT_LEN },
                                            .mass = { DEFAULT_MASS, DEFAULT_MASS } },
                                .angle = { DEFAULT_ANGLE, DEFAULT_ANGLE },
                                .angle_adder = { DEFAULT_ANGLE_ADDER, DEFAULT_ANGLE_ADDER },
                                .ang_vel = { DEFAULT_ANG_VEL, DEFAULT_ANG_VEL } };

    return simulation_init_from_spec(simulation, &spec);
}

double simulation_update_range(Simulation *simulation, size_t start_index, size_t end_index, int steps) {
    if(!simulation || !simulation->state || steps <= 0) {
        return 0.0;
    }

    const size_t pendulum_count = (size_t)TOTAL_PENDULUMS;
    if(end_index > pendulum_count) {
        end_index = pendulum_count;
    }
    if(start_index >= end_index) {
        return 0.0;
    }

    double local_max_ang_vel = 0.0;

    for(size_t i = start_index; i < end_index; ++i) {
        for(int step = 0; step < steps; ++step) {
            pendulum_update(&simulation->state[i], &simulation->params);
        }

        double v0 = fabs(simulation->state[i].ang_vel[0]);
        double v1 = fabs(simulation->state[i].ang_vel[1]);
        if(v0 > local_max_ang_vel)
            local_max_ang_vel = v0;
        if(v1 > local_max_ang_vel)
            local_max_ang_vel = v1;
    }

    return local_max_ang_vel;
}

double simulation_update_steps(Simulation *simulation, int steps) {
    if(!simulation || !simulation->state || steps <= 0) {
        return 0.0;
    }

    return simulation_update_range(simulation, (size_t)0, (size_t)TOTAL_PENDULUMS, steps);
}

void simulation_fill_render_samples(const Simulation *simulation, PendulumRenderSample *out, size_t count) {
    if(!simulation || !simulation->state || !out || count == 0) {
        return;
    }

    const size_t pendulum_count = (size_t)TOTAL_PENDULUMS;
    if(count > pendulum_count) {
        count = pendulum_count;
    }

    for(size_t i = 0; i < count; ++i) {
        out[i].angle[0] = (float)simulation->state[i].angle[0];
        out[i].angle[1] = (float)simulation->state[i].angle[1];
        out[i].ang_vel[0] = (float)simulation->state[i].ang_vel[0];
        out[i].ang_vel[1] = (float)simulation->state[i].ang_vel[1];
    }
}

size_t simulation_get_count(const Simulation *simulation) {
    return simulation && simulation->state ? (size_t)TOTAL_PENDULUMS : (size_t)0;
}

double simulation_get_len(const Simulation *simulation, int rod_index) {
    if(!simulation || rod_index < 0 || rod_index >= 2) {
        return 0.0;
    }

    return simulation->params.len[rod_index];
}

void simulation_quit(Simulation *simulation) {
    if(!simulation) {
        return;
    }

    free(simulation->state);
    simulation->state = NULL;

    simulation->params = (PendulumParams){ 0 };
}

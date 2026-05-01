#include "simulation.h"

#include "../config/config.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
    PendulumParams params;
    double angle[2];
    double angle_adder[2];
    double ang_vel[2];
} SimulationInitSpec;

static double simulation_compute_max_len(const PendulumParams *params) {
    return params->len[0] > params->len[1] ? params->len[0] : params->len[1];
}

static double simulation_compute_initial_max_ang_vel(const double ang_vel[2]) {
    double v0 = fabs(ang_vel[0]);
    double v1 = fabs(ang_vel[1]);
    return v0 > v1 ? v0 : v1;
}

static bool simulation_init_from_spec(Simulation *simulation, const SimulationInitSpec *spec) {
    simulation->params = spec->params;
    simulation->max_len = simulation_compute_max_len(&simulation->params);
    simulation->max_ang_vel = simulation_compute_initial_max_ang_vel(spec->ang_vel);
    simulation->state = NULL;

    simulation->state = (PendulumState *)malloc((size_t)TOTAL_PENDULUMS * sizeof(PendulumState));
    if(simulation->state == NULL) {
        return false;
    }

    for(int i = 0; i < TOTAL_PENDULUMS; ++i) {
        pendulum_state_init(
            &simulation->state[i],
            spec->angle[0] + spec->angle_adder[0] * (double)i,
            spec->ang_vel[0],
            spec->angle[1] + spec->angle_adder[1] * (double)i,
            spec->ang_vel[1]
        );
    }

    return true;
}

bool simulation_init_custom(Simulation *simulation) {
    SimulationInitSpec spec = {
        .params = {
            .len = { CUSTOM_LEN_ROD1, CUSTOM_LEN_ROD2 },
            .mass = { CUSTOM_MASS_ROD1, CUSTOM_MASS_ROD2 }
        },
        .angle = { CUSTOM_ANGLE_ROD1, CUSTOM_ANGLE_ROD2 },
        .angle_adder = { CUSTOM_ANGLE_ADDER_ROD1, CUSTOM_ANGLE_ADDER_ROD2 },
        .ang_vel = { CUSTOM_ANG_VEL_ROD1, CUSTOM_ANG_VEL_ROD2 }
    };

    return simulation_init_from_spec(simulation, &spec);
}

bool simulation_init_default(Simulation *simulation) {
    SimulationInitSpec spec = {
        .params = {
            .len = { DEFAULT_LEN, DEFAULT_LEN },
            .mass = { DEFAULT_MASS, DEFAULT_MASS }
        },
        .angle = { DEFAULT_ANGLE, DEFAULT_ANGLE },
        .angle_adder = { DEFAULT_ANGLE_ADDER, DEFAULT_ANGLE_ADDER },
        .ang_vel = { DEFAULT_ANG_VEL, DEFAULT_ANG_VEL }
    };

    return simulation_init_from_spec(simulation, &spec);
}

double simulation_update_range(
    Simulation *simulation,
    int start_index,
    int end_index,
    int steps
) {
    if(!simulation || !simulation->state || steps <= 0) {
        return 0.0;
    }

    if(start_index < 0) {
        start_index = 0;
    }
    if(end_index > TOTAL_PENDULUMS) {
        end_index = TOTAL_PENDULUMS;
    }
    if(start_index >= end_index) {
        return 0.0;
    }

    double local_max_ang_vel = 0.0;

    for(int i = start_index; i < end_index; ++i) {
        for(int step = 0; step < steps; ++step) {
            pendulum_update(&simulation->state[i], &simulation->params);
        }

        double v0 = fabs(simulation->state[i].ang_vel[0]);
        double v1 = fabs(simulation->state[i].ang_vel[1]);
        if(v0 > local_max_ang_vel) local_max_ang_vel = v0;
        if(v1 > local_max_ang_vel) local_max_ang_vel = v1;
    }

    return local_max_ang_vel;
}

void simulation_update_steps(Simulation *simulation, int steps) {
    if(!simulation || !simulation->state || steps <= 0) {
        return;
    }

    simulation->max_ang_vel = simulation_update_range(simulation, 0, TOTAL_PENDULUMS, steps);
}

void simulation_fill_render_samples(const Simulation *simulation, PendulumRenderSample *out, int count) {
    if(!simulation || !simulation->state || !out || count <= 0) {
        return;
    }

    if(count > TOTAL_PENDULUMS) {
        count = TOTAL_PENDULUMS;
    }

    for(int i = 0; i < count; ++i) {
        out[i].len[0] = (float)simulation->params.len[0];
        out[i].len[1] = (float)simulation->params.len[1];
        out[i].angle[0] = (float)simulation->state[i].angle[0];
        out[i].angle[1] = (float)simulation->state[i].angle[1];
        out[i].ang_vel[0] = (float)simulation->state[i].ang_vel[0];
        out[i].ang_vel[1] = (float)simulation->state[i].ang_vel[1];
    }
}

void simulation_set_max_ang_vel(Simulation *simulation, double max_ang_vel) {
    if(simulation) {
        simulation->max_ang_vel = max_ang_vel;
    }
}

int simulation_get_count(const Simulation *simulation) {
    return simulation && simulation->state ? TOTAL_PENDULUMS : 0;
}

double simulation_get_max_len(const Simulation *simulation) {
    return simulation ? simulation->max_len : 0.0;
}

double simulation_get_max_ang_vel(const Simulation *simulation) {
    return simulation ? simulation->max_ang_vel : 0.0;
}

void simulation_quit(Simulation *simulation) {
    if(!simulation) {
        return;
    }

    free(simulation->state);
    simulation->state = NULL;

    simulation->params = (PendulumParams){ 0 };
    simulation->max_len = 0.0;
    simulation->max_ang_vel = 0.0;
}

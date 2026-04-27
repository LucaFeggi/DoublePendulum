#include "simulation.h"

#include "../config.h"
#include "SDL.h"

#include <math.h>
#include <stdlib.h>

#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
typedef struct {
    Pendulum *pendulums;
    double *thread_max_ang_vel;
} SimulationUpdateJob;

static void simulation_update_job(void *context, int start_index, int end_index, int worker_id) {
    SimulationUpdateJob *job = (SimulationUpdateJob *)context;
    double local_max_ang_vel = 0.0;

    for(int i = start_index; i < end_index; ++i) {
        pendulum_update(&job->pendulums[i]);

        double v0 = fabs(job->pendulums[i].rod[0].ang_vel);
        double v1 = fabs(job->pendulums[i].rod[1].ang_vel);
        if(v0 > local_max_ang_vel) local_max_ang_vel = v0;
        if(v1 > local_max_ang_vel) local_max_ang_vel = v1;
    }

    job->thread_max_ang_vel[worker_id] = local_max_ang_vel;
}

static double simulation_reduce_max_ang_vel(const Simulation *simulation) {
    double max_ang_vel = 0.0;
    int num_threads = threadpool_get_num_threads(&simulation->threadpool);

    for(int i = 0; i < num_threads; ++i) {
        if(simulation->thread_max_ang_vel[i] > max_ang_vel) {
            max_ang_vel = simulation->thread_max_ang_vel[i];
        }
    }

    return max_ang_vel;
}

static bool simulation_init_threadpool(Simulation *simulation, int worker_threads, const char *caller_name) {
    if(!threadpool_init(&simulation->threadpool, worker_threads)) {
        SDL_Log("ERROR: %s: Failed to initialize thread pool.", caller_name);
        return false;
    }

    int num_threads = threadpool_get_num_threads(&simulation->threadpool);
    simulation->thread_max_ang_vel = (double *)calloc((size_t)num_threads, sizeof(double));
    if(!simulation->thread_max_ang_vel) {
        SDL_Log("ERROR: %s: Failed to allocate thread-local angular velocity buffer.", caller_name);
        threadpool_quit(&simulation->threadpool);
        return false;
    }

    return true;
}
#endif

bool simulation_init_custom(Simulation *simulation, int worker_threads) {
    (void)worker_threads;

    simulation->pendulum = (Pendulum *)malloc(TOTAL_PENDULUMS * sizeof(Pendulum));
    if(simulation->pendulum == NULL) {
        SDL_Log("ERROR: simulation_init_custom: Failed to allocate memory for pendulums.");
        return false;
    }

    simulation->max_len = CUSTOM_LEN_ROD1 > CUSTOM_LEN_ROD2 ? CUSTOM_LEN_ROD1 : CUSTOM_LEN_ROD2;
    simulation->max_ang_vel = CUSTOM_ANG_VEL_ROD1 > CUSTOM_ANG_VEL_ROD2 ? CUSTOM_ANG_VEL_ROD1 : CUSTOM_ANG_VEL_ROD2;
    double angle_adder_rod1 = 0.0;
    double angle_adder_rod2 = 0.0;
    for(int i = 0; i < TOTAL_PENDULUMS; i++){
        pendulum_init_custom(
            &simulation->pendulum[i],
            CUSTOM_ANGLE_ROD1 + angle_adder_rod1, CUSTOM_ANG_VEL_ROD1, CUSTOM_LEN_ROD1, CUSTOM_MASS_ROD1,
            CUSTOM_ANGLE_ROD2 + angle_adder_rod2, CUSTOM_ANG_VEL_ROD2, CUSTOM_LEN_ROD2, CUSTOM_MASS_ROD2
        );
        angle_adder_rod1 += CUSTOM_ANGLE_ADDER_ROD1;
        angle_adder_rod2 += CUSTOM_ANGLE_ADDER_ROD2;
    }

#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
    if(!simulation_init_threadpool(simulation, worker_threads, "simulation_init_custom")) {
        free(simulation->pendulum);
        return false;
    }
#endif

    return true;
}

bool simulation_init_default(Simulation *simulation, int worker_threads) {
    (void)worker_threads;

    simulation->pendulum = (Pendulum *)malloc(TOTAL_PENDULUMS * sizeof(Pendulum));
    if(simulation->pendulum == NULL) {
        SDL_Log("ERROR: simulation_init_default: Failed to allocate memory for pendulums.");
        return false;
    }

    simulation->max_len = DEFAULT_LEN;
    simulation->max_ang_vel = DEFAULT_ANG_VEL;
    double angle_adder = 0.0;
    for(int i = 0; i < TOTAL_PENDULUMS; i++){
        // Assuming pendulum_init_default does not fail critically
        pendulum_init_default(&simulation->pendulum[i], DEFAULT_ANGLE + angle_adder);
        angle_adder += DEFAULT_ANGLE_ADDER;
    }

#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
    if(!simulation_init_threadpool(simulation, worker_threads, "simulation_init_default")) {
        free(simulation->pendulum);
        return false;
    }
#endif

    return true;
}

ThreadPool *simulation_get_threadpool(Simulation *simulation) {
#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
    return &simulation->threadpool;
#else
    (void)simulation;
    return NULL;
#endif
}

void simulation_update(Simulation *simulation){
#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
    SimulationUpdateJob job = {
        .pendulums = simulation->pendulum,
        .thread_max_ang_vel = simulation->thread_max_ang_vel
    };

	threadpool_parallel_for(&simulation->threadpool, TOTAL_PENDULUMS, simulation_update_job, &job);
	simulation->max_ang_vel = simulation_reduce_max_ang_vel(simulation);
#else
    double max_ang_vel = 0.0;
	for(int i = 0; i < TOTAL_PENDULUMS; i++) {
		pendulum_update(&simulation->pendulum[i]);  // pass pointer to current element
		double v0 = fabs(simulation->pendulum[i].rod[0].ang_vel);
		double v1 = fabs(simulation->pendulum[i].rod[1].ang_vel);
		if(v0 > max_ang_vel) max_ang_vel = v0;
		if(v1 > max_ang_vel) max_ang_vel = v1;
	}
    simulation->max_ang_vel = max_ang_vel;
#endif
}

void simulation_quit(Simulation *simulation){
#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
	threadpool_quit(&simulation->threadpool);
    free(simulation->thread_max_ang_vel);
#endif
	free(simulation->pendulum);
}

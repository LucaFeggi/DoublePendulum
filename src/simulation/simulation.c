#include "simulation.h"

#include "../config.h"
#include "SDL.h"

bool simulation_init_custom(Simulation *simulation) {
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
            CUSTOM_ANGLE_ROD1 + angle_adder_rod1, CUSTOM_ANG_VEL_ROD1, CUSTOM_ANG_ACC_ROD1, CUSTOM_LEN_ROD1, CUSTOM_MASS_ROD1,
            CUSTOM_ANGLE_ROD2 + angle_adder_rod2, CUSTOM_ANG_VEL_ROD2, CUSTOM_ANG_ACC_ROD2, CUSTOM_LEN_ROD2, CUSTOM_MASS_ROD2
        );
        angle_adder_rod1 += CUSTOM_ANGLE_ADDER_ROD1;
        angle_adder_rod2 += CUSTOM_ANGLE_ADDER_ROD2;
    }

#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
    if(!threadpool_init(&simulation->threadpool)) {
        SDL_Log("ERROR: simulation_init_custom: Failed to initialize thread pool.");
        free(simulation->pendulum);
        return false;
    }
#endif

    return true;
}

bool simulation_init_default(Simulation *simulation) {
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
    if(!threadpool_init(&simulation->threadpool)) {
        SDL_Log("ERROR: simulation_init_default: Failed to initialize thread pool.");
        free(simulation->pendulum);
        return false;
    }
#endif

    return true;
}

void simulation_update(Simulation *simulation){
    // COLOR_DECAY should be in renderer somewhere. Because is rendering stuff, not simulation
    // but i understand this is simple here
	simulation->max_ang_vel *= COLOR_DECAY;		// Decaying before updating so old peaks fade 
#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
	threadpool_run(&simulation->threadpool, simulation->pendulum);	// pendulum_update in multithread
	simulation->max_ang_vel = threadpool_get_max_ang_vel(&simulation->threadpool);
#else
	for(int i = 0; i < TOTAL_PENDULUMS; i++) {
		pendulum_update(&simulation->pendulum[i]);  // pass pointer to current element
		double v0 = fabs(simulation->pendulum[i].rod[0].ang_vel);
		double v1 = fabs(simulation->pendulum[i].rod[1].ang_vel);
		if(v0 > simulation->max_ang_vel) simulation->max_ang_vel = v0;
		if(v1 > simulation->max_ang_vel) simulation->max_ang_vel = v1;
	}
#endif
}

void simulation_quit(Simulation *simulation){
#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
	threadpool_quit(&simulation->threadpool);
#endif
	free(simulation->pendulum);
}
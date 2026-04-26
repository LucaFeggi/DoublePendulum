#ifndef SIMULATION_H
#define SIMULATION_H

#include "pendulum.h"
#include "../app/threadpool.h"
#include "../config.h"

typedef struct{
	Pendulum *pendulum;
	double max_len;				// max length (never updates) among all the nodes. Used to render correctly different length nodes.
	double max_ang_vel;			// max angular velocity (always updates) ever register in the current simulation. Used for coloring.
#if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
	ThreadPool threadpool;		// CPU mulithtreading turns on if TOTAL_PENDULUMS > MULTITHREADING_THRESHOLD
	double *thread_max_ang_vel;
#endif
}Simulation;

bool simulation_init_custom(Simulation *simulation);
bool simulation_init_default(Simulation *simulation);
void simulation_update(Simulation *simulation);
void simulation_quit(Simulation *simulation);

#endif

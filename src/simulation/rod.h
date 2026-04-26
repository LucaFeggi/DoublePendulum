#ifndef SIMULATION_ROD_H
#define SIMULATION_ROD_H

typedef struct{
	double len;			// length
	double angle;
	double ang_vel;     // angular velocity
	double ang_acc;     // angular acceleration
	double mass;
}Rod;

void rod_init_custom(Rod *rod, double angle, double ang_vel, double ang_acc, double len, double mass);
void rod_init_default(Rod *rod, double angle);

#endif // SIMULATION_ROD_H

#ifndef CONFIG_SIMULATION_CONFIG_H
#define CONFIG_SIMULATION_CONFIG_H

// ----- Physics parameters -----

#define PENDULUM_INIT_MODE 0 // 0 -> default, 1 -> custom

// The default rods are both equally initialized, so just one group of defines.
#define DEFAULT_ANG_VEL 0.0
#define DEFAULT_LEN 100
#define DEFAULT_MASS 10
#define DEFAULT_ANGLE 2.4
#define DEFAULT_ANGLE_ADDER 0.001

// The 2 custom rods can be different to each other, so 2 groups of defines.
#define CUSTOM_ANG_VEL_ROD1 0.0
#define CUSTOM_LEN_ROD1 100
#define CUSTOM_MASS_ROD1 10
#define CUSTOM_ANGLE_ROD1 1.28
#define CUSTOM_ANGLE_ADDER_ROD1 0.001

#define CUSTOM_ANG_VEL_ROD2 0.0
#define CUSTOM_LEN_ROD2 50
#define CUSTOM_MASS_ROD2 5
#define CUSTOM_ANGLE_ROD2 2.28
#define CUSTOM_ANGLE_ADDER_ROD2 0.001

#define GRAVITY_CENTI 981
#define G ((double)GRAVITY_CENTI / 100.0)

#define SIMULATION_STEPS_PER_SECOND 1000
#define SIMULATION_DT (1.0 / (double)SIMULATION_STEPS_PER_SECOND)
#define SIMULATION_TIME_SCALE 3

#define TOTAL_PENDULUMS 2000

// ----- Performance parameters -----

#define MULTITHREADING_THRESHOLD 500

#endif // CONFIG_SIMULATION_CONFIG_H

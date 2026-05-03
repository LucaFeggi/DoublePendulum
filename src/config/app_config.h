#ifndef CONFIG_APP_CONFIG_H
#define CONFIG_APP_CONFIG_H

// Max fixed simulation steps processed before a frame is rendered.
#define MAX_SIMULATION_STEPS_PER_FRAME 32

// Minimum loop items per active job. Also controls how many worker threads are
// useful for TOTAL_PENDULUMS before the app caps the pool by CPU count.
#define THREADPOOL_MIN_ITEMS_PER_JOB 256

#endif // CONFIG_APP_CONFIG_H

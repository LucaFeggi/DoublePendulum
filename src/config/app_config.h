#ifndef CONFIG_APP_CONFIG_H
#define CONFIG_APP_CONFIG_H

#include "simulation_config.h"

// Lowest render FPS where the app should still try to keep the configured
// simulation speed before the per-frame safety cap starts dropping time.
#define MIN_SUPPORTED_RENDER_FPS 60

// Max fixed simulation steps processed before a frame is rendered.
// Derived as ceil(SIMULATION_STEPS_PER_SECOND * SIMULATION_TIME_SCALE / MIN_SUPPORTED_RENDER_FPS).
#define MAX_SIMULATION_STEPS_PER_FRAME                                                                                 \
    ((SIMULATION_STEPS_PER_SECOND * SIMULATION_TIME_SCALE + MIN_SUPPORTED_RENDER_FPS - 1) / MIN_SUPPORTED_RENDER_FPS)

// Minimum loop items per active job. Also controls how many worker threads are
// useful for TOTAL_PENDULUMS before the app caps the pool by CPU count.
#define THREADPOOL_MIN_ITEMS_PER_JOB 256

#endif // CONFIG_APP_CONFIG_H

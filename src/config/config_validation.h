#ifndef CONFIG_CONFIG_VALIDATION_H
#define CONFIG_CONFIG_VALIDATION_H

#include "app_config.h"
#include "simulation_config.h"
#include "render_config.h"

_Static_assert(TOTAL_PENDULUMS >= 1, "TOTAL_PENDULUMS must be >= 1");

_Static_assert(PENDULUM_INIT_MODE == 0 || PENDULUM_INIT_MODE == 1, "PENDULUM_INIT_MODE must be 0 or 1");

_Static_assert(GRAVITY_CENTI > 0, "GRAVITY_CENTI must be > 0");
_Static_assert(SIMULATION_STEPS_PER_SECOND > 0, "SIMULATION_STEPS_PER_SECOND must be > 0");
_Static_assert(SIMULATION_TIME_SCALE > 0, "SIMULATION_TIME_SCALE must be > 0");
_Static_assert(MIN_SUPPORTED_RENDER_FPS > 0, "MIN_SUPPORTED_RENDER_FPS must be > 0");
_Static_assert(MAX_SIMULATION_STEPS_PER_FRAME >= 1, "MAX_SIMULATION_STEPS_PER_FRAME must be >= 1");

_Static_assert(DEFAULT_LEN > 0, "DEFAULT_LEN must be > 0");
_Static_assert(DEFAULT_MASS > 0, "DEFAULT_MASS must be > 0");

_Static_assert(CUSTOM_LEN_ROD1 > 0, "CUSTOM_LEN_ROD1 must be > 0");
_Static_assert(CUSTOM_LEN_ROD2 > 0, "CUSTOM_LEN_ROD2 must be > 0");
_Static_assert(CUSTOM_MASS_ROD1 > 0, "CUSTOM_MASS_ROD1 must be > 0");
_Static_assert(CUSTOM_MASS_ROD2 > 0, "CUSTOM_MASS_ROD2 must be > 0");

_Static_assert(THREADPOOL_MIN_ITEMS_PER_JOB >= 1, "THREADPOOL_MIN_ITEMS_PER_JOB must be >= 1");

_Static_assert(ROD_WIDTH_PER_MILLE > 0, "ROD_WIDTH_PER_MILLE must be > 0");
_Static_assert(TRAIL == 0 || TRAIL == 1, "TRAIL must be 0 or 1");
_Static_assert(TRAIL_WIDTH_PER_MILLE > 0, "TRAIL_WIDTH_PER_MILLE must be > 0");
_Static_assert(TRAIL_DURATION_MILLISECONDS > 0, "TRAIL_DURATION_MILLISECONDS must be > 0");
_Static_assert(TRAIL_BUCKET_MILLISECONDS > 0, "TRAIL_BUCKET_MILLISECONDS must be > 0");
_Static_assert(TRAIL_BUCKET_COUNT >= 1, "TRAIL_BUCKET_COUNT must be >= 1");
_Static_assert(TRAIL_FADE_GAMMA_PER_MILLE > 0, "TRAIL_FADE_GAMMA_PER_MILLE must be > 0");
_Static_assert(COLOR_DECAY_PER_MILLE >= 0 && COLOR_DECAY_PER_MILLE <= 1000,
               "COLOR_DECAY_PER_MILLE must be in [0, 1000]");
_Static_assert(COLOR_DECAY_REFERENCE_FPS > 0, "COLOR_DECAY_REFERENCE_FPS must be > 0");

#endif // CONFIG_CONFIG_VALIDATION_H

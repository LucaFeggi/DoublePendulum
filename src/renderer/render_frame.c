#include "render_frame.h"

#include "../config/render_config.h"
#include "../config/simulation_config.h"

#include <math.h>
#include <stdlib.h>

static float render_frame_compute_max_len(const float len[2]) {
    return len[0] > len[1] ? len[0] : len[1];
}

static float render_frame_compute_max_ang_vel(const PendulumRenderSample *samples, size_t count) {
    float max_ang_vel = 0.0f;

    if(!samples) {
        return max_ang_vel;
    }

    for(size_t i = 0; i < count; ++i) {
        float v0 = fabsf(samples[i].ang_vel[0]);
        float v1 = fabsf(samples[i].ang_vel[1]);
        if(v0 > max_ang_vel)
            max_ang_vel = v0;
        if(v1 > max_ang_vel)
            max_ang_vel = v1;
    }

    return max_ang_vel;
}

bool render_frame_init(RenderFrame *rf, const Simulation *sim) {
    rf->len[0] = (float)simulation_get_len(sim, 0);
    rf->len[1] = (float)simulation_get_len(sim, 1);
    rf->max_len = render_frame_compute_max_len(rf->len);
    rf->max_ang_vel = 0.0f;
    rf->pen_data = (PendulumRenderSample *)malloc((size_t)TOTAL_PENDULUMS * sizeof(PendulumRenderSample));
    if(!rf->pen_data) {
        return false;
    }

    simulation_fill_render_samples(sim, rf->pen_data, (size_t)TOTAL_PENDULUMS);
    rf->max_ang_vel = render_frame_compute_max_ang_vel(rf->pen_data, (size_t)TOTAL_PENDULUMS);
    return true;
}

void render_frame_quit(RenderFrame *rf) {
    if(!rf) {
        return;
    }

    free(rf->pen_data);
    rf->pen_data = NULL;
    rf->len[0] = 0.0f;
    rf->len[1] = 0.0f;
    rf->max_len = 0.0f;
    rf->max_ang_vel = 0.0f;
}

static float render_frame_color_decay_for_delta_time(float delta_time) {
    if(!(delta_time > 0.0f)) {
        return 1.0f;
    }

    return powf((float)COLOR_DECAY, delta_time * (float)COLOR_DECAY_REFERENCE_FPS);
}

void render_frame_pack(RenderFrame *rf, const Simulation *sim, float current_max_ang_vel, float delta_time) {
    float decayed_max_ang_vel = rf->max_ang_vel * render_frame_color_decay_for_delta_time(delta_time);
    rf->max_ang_vel = current_max_ang_vel > decayed_max_ang_vel ? current_max_ang_vel : decayed_max_ang_vel;

    simulation_fill_render_samples(sim, rf->pen_data, (size_t)TOTAL_PENDULUMS);
}

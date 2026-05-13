#include "color.h"

#include <math.h>

#define SPECTRUM_STAGES 8

static const SDL_Color spectrum[SPECTRUM_STAGES] = {
    { 255, 56, 0, 255 },    // Late M Deep Red
    { 255, 204, 111, 255 }, // M-type Orange-Red
    { 255, 210, 161, 255 }, // K-type Light Orange
    { 255, 244, 234, 255 }, // G-type Yellow-White
    { 248, 247, 255, 255 }, // F-type Pale Yellow-White
    { 202, 215, 255, 255 }, // A-type White
    { 170, 191, 255, 255 }, // B-type Blue-White
    { 155, 176, 255, 255 }  // O-type Blue
};

static float color_clamp_unit(float t) {
    if(!(t >= 0.0f)) {
        return 0.0f;
    }
    if(t > 1.0f) {
        return 1.0f;
    }
    return t;
}

static float color_normalize_speed(float speed, float max_speed) {
    if(!(max_speed > 0.0f)) {
        return 0.0f;
    }
    return color_clamp_unit(speed / max_speed);
}

static SDL_Color color_interpolate(SDL_Color c1, SDL_Color c2, float frac) {
    SDL_Color result;
    result.r = (Uint8)(c1.r + (c2.r - c1.r) * frac);
    result.g = (Uint8)(c1.g + (c2.g - c1.g) * frac);
    result.b = (Uint8)(c1.b + (c2.b - c1.b) * frac);
    result.a = 255;
    return result;
}

static SDL_Color color_map(float t) {
    float clamped_t = color_clamp_unit(t);
    float pos = clamped_t * (SPECTRUM_STAGES - 1);
    int idx = (int)pos;
    float frac = pos - idx;

    if(idx >= SPECTRUM_STAGES - 1) {
        return spectrum[SPECTRUM_STAGES - 1];
    }

    return color_interpolate(spectrum[idx], spectrum[idx + 1], frac);
}

void color_get_double_pendulum(const PendulumRenderSample *pen, const float len[2], float max_ang_vel,
                               const PendulumRenderTrig *trig, SDL_Color color[2]) {
    if(color == NULL) {
        return;
    }

    if(pen == NULL || len == NULL) {
        color[0] = color_map(0.0f);
        color[1] = color_map(0.0f);
        return;
    }

    PendulumRenderTrig local_trig;
    if(trig == NULL) {
        local_trig.sin0 = sinf(pen->angle[0]);
        local_trig.cos0 = cosf(pen->angle[0]);
        local_trig.sin1 = sinf(pen->angle[1]);
        local_trig.cos1 = cosf(pen->angle[1]);
        trig = &local_trig;
    }

    float len0 = len[0];
    float len1_com = len[1] * 0.5f;
    float omega0 = pen->ang_vel[0];
    float omega1 = pen->ang_vel[1];

    float v1 = fabsf(omega0) * fabsf(len0 * 0.5f);

    float vx_pivot2 = omega0 * len0 * trig->cos0;
    float vy_pivot2 = -omega0 * len0 * trig->sin0;

    float vx_com2 = omega1 * len1_com * trig->cos1;
    float vy_com2 = -omega1 * len1_com * trig->sin1;

    float vx_total = vx_pivot2 + vx_com2;
    float vy_total = vy_pivot2 + vy_com2;
    float v2 = sqrtf(vx_total * vx_total + vy_total * vy_total);

    float max_omega = fabsf(max_ang_vel);
    float v1_max = max_omega * fabsf(len0 * 0.5f);
    float v2_max = max_omega * (fabsf(len0) + fabsf(len1_com));

    color[0] = color_map(color_normalize_speed(v1, v1_max));
    color[1] = color_map(color_normalize_speed(v2, v2_max));
}

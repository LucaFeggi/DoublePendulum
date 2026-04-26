#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include "../../app/render_data.h"

#include "SDL.h"

#define SPECTRUM_STAGES 8

// Stellar color palette (red:slow -> blue:fast)
const SDL_Color spectrum[SPECTRUM_STAGES] = {
    {255, 56, 0, 255},      // Late M Deep Red
    {255, 204, 111, 255},   // M-type Orange-Red
    {255, 210, 161, 255},   // K-type Light Orange
    {255, 244, 234, 255},   // G-type Yellow-White
    {248, 247, 255, 255},   // F-type Pale Yellow-White
    {202, 215, 255, 255},   // A-type White
    {170, 191, 255, 255},   // B-type Blue-White
    {155, 176, 255, 255}    // O-type Blue
};

static inline SDL_Color interpolate(SDL_Color c1, SDL_Color c2, float frac) {
    SDL_Color result;
    result.r = (Uint8)(c1.r + (c2.r - c1.r) * frac);
    result.g = (Uint8)(c1.g + (c2.g - c1.g) * frac);
    result.b = (Uint8)(c1.b + (c2.b - c1.b) * frac);
    result.a = 255;
    return result;
}

static inline SDL_Color color_map(float t) {
    if(t < 0.0f){
        t = 0.0f;
    }
    else if(t > 1.0f){
        t = 1.0f;
    }
    float pos = t * (SPECTRUM_STAGES - 1);
    int idx = (int)pos;
    float frac = pos - idx;
    if(idx >= SPECTRUM_STAGES - 1){
        return spectrum[SPECTRUM_STAGES - 1];
    }
    return interpolate(spectrum[idx], spectrum[idx + 1], frac);
}


// floats or double? 
void color_get_double_pendulum(PendulumRenderData *pen, double max_ang_vel, SDL_Color color[2]){
    float v1 = fabs(pen->ang_vel[0]) * (pen->len[0] / 2.0);    // Tangential velocity at the center of mass (COM) of the first rod

    float vx_pivot2 = -pen->ang_vel[0] * pen->len[0] * sin(pen->angle[0]); // Velocity of the pivot of rod2 (tip of rod1)
    float vy_pivot2 = pen->ang_vel[0] * pen->len[0] * cos(pen->angle[0]);

    float vx_com2 = -pen->ang_vel[1] * pen->len[1] * sin(pen->angle[1]);   // Velocity of rod2 COM relative to its pivot
    float vy_com2 = pen->ang_vel[1] * pen->len[1] * cos(pen->angle[1]);

    float vx_total = vx_pivot2 + vx_com2;  // Total velocity of rod2 COM
    float vy_total = vy_pivot2 + vy_com2;
    float v2 = sqrt(vx_total * vx_total + vy_total * vy_total);

    float v1_max = max_ang_vel * (pen->len[0] / 2.0);  // Maximum tangential velocity of rod1 COM (used for normalization)
    float v2_max = max_ang_vel * pen->len[0] + max_ang_vel * (pen->len[1] / 2.0);  // Maximum COM velocity of rod2 (tip of rod1 + rod2 COM relative to its pivot)

    float t1 = (v1 / v1_max);    // Normalize to 0..1 and clamp
    float t2 = (v2 / v2_max);

    if(t1 > 1.0f){
        t1 = 1.0f;
    }
    if(t2 > 1.0f){
        t2 = 1.0f;
    }

    color[0] = color_map(t1);   // Map normalized velocities to colors
    color[1] = color_map(t2);
}

#endif // !COLOR_UTILS_H
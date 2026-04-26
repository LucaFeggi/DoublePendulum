#include "rk4.h"

#include "../config.h"

// Runge-Kutta 4th order method to resolve ODEs as accurately as possible

// Structs for angular states and respective derived_states.
typedef struct{
    double angle[2];    // Remember: in a double pendulum there are [2] rods
    double ang_vel[2];
}State;

typedef struct{
    double d_angle[2];
    double d_ang_vel[2];
}Derivative;

// Computing angular acceleration of the 2 rods (double pendulum's specific formulas)
static inline double comput_ang_acc_0(Pendulum *pendulum, State state){
    double m0 = pendulum->rod[0].mass;
    double m1 = pendulum->rod[1].mass;
    double l0 = pendulum->rod[0].len;
    double l1 = pendulum->rod[1].len;

    double theta0 = state.angle[0];
    double theta1 = state.angle[1];
    double omega0 = state.ang_vel[0];
    double omega1 = state.ang_vel[1];

    double delta = theta0 - theta1;
    double numerator = - G * (2 * m0 + m1) * sin(theta0) - m1 * G * sin(theta0 - 2 * theta1) - 2 * sin(delta) * m1 * (omega1 * omega1 * l1 + omega0 * omega0 * l0 * cos(delta));
    double denominator = l0 * (2 * m0 + m1 - m1 * cos(2 * delta));

    return (numerator / denominator);
}

static inline double comput_ang_acc_1(Pendulum *pendulum, State state){
    double m0 = pendulum->rod[0].mass;
    double m1 = pendulum->rod[1].mass;
    double l0 = pendulum->rod[0].len;
    double l1 = pendulum->rod[1].len;

    double theta0 = state.angle[0];
    double theta1 = state.angle[1];
    double omega0 = state.ang_vel[0];
    double omega1 = state.ang_vel[1];

    double delta = theta0 - theta1;
    double numerator = 2 * sin(delta) * (omega0 * omega0 * l0 * (m0 + m1) + G * (m0 + m1) * cos(theta0) + omega1 * omega1 * l1 * m1 * cos(delta));
    double denominator = l1 * (2 * m0 + m1 - m1 * cos(2 * delta));

    return (numerator / denominator);
}

static inline Derivative evaluate(Pendulum *pendulum, State state, double dt, Derivative d){
    State s_next;   // initial values
    s_next.angle[0] = state.angle[0] + d.d_angle[0] * dt;
    s_next.angle[1] = state.angle[1] + d.d_angle[1] * dt;
    s_next.ang_vel[0] = state.ang_vel[0] + d.d_ang_vel[0] * dt;
    s_next.ang_vel[1] = state.ang_vel[1] + d.d_ang_vel[1] * dt;

    Derivative output;  // deriving them
    output.d_angle[0] = s_next.ang_vel[0];
    output.d_angle[1] = s_next.ang_vel[1];
    output.d_ang_vel[0] = comput_ang_acc_0(pendulum, s_next);   // Remember: acceleration is the derived of velocity
    output.d_ang_vel[1] = comput_ang_acc_1(pendulum, s_next);

    return output;
}

void rk4_pendulum_update(Pendulum *pendulum){
    State state;
    state.angle[0] = pendulum->rod[0].angle;
    state.angle[1] = pendulum->rod[1].angle;
    state.ang_vel[0] = pendulum->rod[0].ang_vel;
    state.ang_vel[1] = pendulum->rod[1].ang_vel;

    Derivative k1 = evaluate(pendulum, state, 0.0, (Derivative){ 0 });
    Derivative k2 = evaluate(pendulum, state, DT * SPEED_FACTOR * 0.5, k1);
    Derivative k3 = evaluate(pendulum, state, DT * SPEED_FACTOR * 0.5, k2);
    Derivative k4 = evaluate(pendulum, state, DT * SPEED_FACTOR, k3);
    
    pendulum->rod[0].angle += (DT * SPEED_FACTOR) / 6.0 * (k1.d_angle[0] + 2 * k2.d_angle[0] + 2 * k3.d_angle[0] + k4.d_angle[0]);
    pendulum->rod[0].ang_vel += (DT * SPEED_FACTOR) / 6.0 * (k1.d_ang_vel[0] + 2 * k2.d_ang_vel[0] + 2 * k3.d_ang_vel[0] + k4.d_ang_vel[0]);
    pendulum->rod[1].angle += (DT * SPEED_FACTOR) / 6.0 * (k1.d_angle[1] + 2 * k2.d_angle[1] + 2 * k3.d_angle[1] + k4.d_angle[1]);
    pendulum->rod[1].ang_vel += (DT * SPEED_FACTOR) / 6.0 * (k1.d_ang_vel[1] + 2 * k2.d_ang_vel[1] + 2 * k3.d_ang_vel[1] + k4.d_ang_vel[1]);
}
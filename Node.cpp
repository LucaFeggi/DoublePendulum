#define _USE_MATH_DEFINES
#include <cmath>

#include "Node.hpp"

Node::Node(int *x0, int *y0, double Length, double Mass, double InitialAngle){
	this->x0 = x0;
	this->y0 = y0;
	this->Mass = Mass;
	this->Length = Length;
	this->Angle = InitialAngle;
	AngularVel = 0.0f;
	AngularAcc = 0.0f;
	x1 = *this->x0 + Length*sin(Angle);
	y1 = *this->y0 + Length*cos(Angle);
}

#pragma once

class Node{
	public:
		Node(int *x0, int *y0, long double Length = 100.0f, long double Mass = 5.0f, long double InitialAngle = M_PI/4.0f);
		int *x0, *y0;
		int x1, y1;
		long double Length;
		long double Mass;
		long double Angle;
		long double AngularVel;
		long double AngularAcc;
};

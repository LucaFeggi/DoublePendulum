#pragma once

class Node{
	public:
		Node(int *x0, int *y0, double Length = 100.0f, double Mass = 5.0f, double InitialAngle = M_PI/4.0f);
		int *x0, *y0;
		int x1, y1;
		double Length;
		double Mass;
		double Angle;
		double AngularVel;
		double AngularAcc;
};

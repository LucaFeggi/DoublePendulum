#include <iostream>
#include <cmath>

#include "DoublePendulum.hpp"
#include "DrawingFunctions.hpp"

DoublePendulum::DoublePendulum(int WindowWidth, int WindowHeight, double FPS, SDL_Surface *Surface, double InitialAngle){
	this->WindowWidth = WindowWidth;
	this->Surface = Surface;
	this->x = WindowWidth/2;
	this->y = WindowHeight/2;
	g = 9.81/FPS;
	FirstNode = new Node(&this->x, &this->y, 100.0, 10.0, InitialAngle);
	SecondNode = new Node(&(FirstNode)->x1, &(FirstNode)->y1, 100.0, 10.0, InitialAngle);
	if(FirstNode->Length + SecondNode->Length >= WindowHeight/2){
		std::cout << "Invalid node length!\nThe sum of all the nodes length must be less than WindowHeight/2.\n";
	}
}

DoublePendulum::~DoublePendulum(){
  delete FirstNode;
  delete SecondNode;
	SDL_FreeSurface(Surface);
}

void DoublePendulum::Erase(){
	DrawLine(*FirstNode->x0, *FirstNode->y0, FirstNode->x1,FirstNode->y1, Surface, WindowWidth, Black, LessWhite);
	DrawLine(*SecondNode->x0, *SecondNode->y0, SecondNode->x1,SecondNode->y1, Surface, WindowWidth, Black, LessWhite);
	DrawPixel(SecondNode->x1, SecondNode->y1, Surface, WindowWidth, LessWhite);
}

bool DoublePendulum::IsAccelerated(){
	if(FirstNode->AngularAcc < -1.001*g || FirstNode->AngularAcc > 1.001*g || SecondNode->AngularAcc < -1.001*g || SecondNode->AngularAcc > 1.001*g)
		return true;
	return false;
}

void DoublePendulum::Update(){

//Lagrange equation solved for angular acceleration using Euler-Lagrange formula
	double Num1 = -sin(FirstNode->Angle-SecondNode->Angle)*(SecondNode->Mass*FirstNode->Length*FirstNode->AngularVel*FirstNode->AngularVel*cos(FirstNode->Angle-SecondNode->Angle)+SecondNode->Mass*SecondNode->Length*SecondNode->AngularVel*SecondNode->AngularVel);
	double Num2 = -g*((FirstNode->Mass+SecondNode->Mass)*sin(FirstNode->Angle)-SecondNode->Mass*sin(SecondNode->Angle)*cos(FirstNode->Angle-SecondNode->Angle));
	double Den = FirstNode->Length*(FirstNode->Mass + SecondNode->Mass*sin(FirstNode->Angle-SecondNode->Angle)*sin(FirstNode->Angle-SecondNode->Angle));
	FirstNode->AngularAcc = (Num1 + Num2) / Den;
	
	Num1 = sin(FirstNode->Angle-SecondNode->Angle)*((FirstNode->Mass+SecondNode->Mass)*FirstNode->Length*FirstNode->AngularVel*FirstNode->AngularVel+SecondNode->Mass*SecondNode->Length*SecondNode->AngularVel*SecondNode->AngularVel*cos(FirstNode->Angle-SecondNode->Angle));
	Num2 = g*((FirstNode->Mass+SecondNode->Mass)*sin(FirstNode->Angle)*cos(FirstNode->Angle-SecondNode->Angle)-(FirstNode->Mass+SecondNode->Mass)*sin(SecondNode->Angle));
	Den = SecondNode->Length*(FirstNode->Mass + SecondNode->Mass*sin(FirstNode->Angle-SecondNode->Angle)*sin(FirstNode->Angle-SecondNode->Angle));
	SecondNode->AngularAcc = (Num1 + Num2) / Den;

	FirstNode->AngularVel += FirstNode->AngularAcc;
	SecondNode->AngularVel += SecondNode->AngularAcc;
 	FirstNode->Angle += FirstNode->AngularVel;
	SecondNode->Angle += SecondNode->AngularVel;

//This is to have some kind of friction, like air resistence. Put a number < 1.
	//FirstNode->AngularVel *= 0.999f;
	//SecondNode->AngularVel *= 0.999f;

	FirstNode->x1 = *FirstNode->x0 + FirstNode->Length*sin(FirstNode->Angle);
	FirstNode->y1 = *FirstNode->y0 + FirstNode->Length*cos(FirstNode->Angle);
	SecondNode->x1 = *SecondNode->x0 + SecondNode->Length*sin(SecondNode->Angle);
	SecondNode->y1 = *SecondNode->y0 + SecondNode->Length*cos(SecondNode->Angle);

}

void DoublePendulum::Draw(){
	DrawLine(*FirstNode->x0, *FirstNode->y0, FirstNode->x1, FirstNode->y1, Surface, WindowWidth, White, LessWhite);
	DrawLine(*SecondNode->x0, *SecondNode->y0,SecondNode->x1, SecondNode->y1, Surface, WindowWidth, White, LessWhite);
	DrawPixel(SecondNode->x1, SecondNode->y1, Surface, WindowWidth, LessWhite);
}

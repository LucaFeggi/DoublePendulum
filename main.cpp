#include "Simulation.hpp"

//-o main DrawingFunctions.cpp Node.cpp DoublePendulum.cpp Simulation.cpp

//The Euler-Lagrange method is good only for short time frames.
//During the simulation, some pendulums might get an enormous amount of angular acceleration, making the program eventually crash.
//To prevent this, the pendulums with a high acceleration will be deleted.

int main(int argc, char** argv){

	Simulation MySimulation(10);

	MySimulation.Cycle();

	return 0;

}

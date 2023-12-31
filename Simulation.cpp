#include <cmath>

#include "Simulation.hpp"
#include <iostream>
Simulation::Simulation(int PendulumsNumber){
	WindowWidth = 1280;
	WindowHeight = 720;
	FPS = 30.0;
	this->PendulumsNumber = PendulumsNumber;
	SDL_Init(SDL_INIT_VIDEO);
	Window = SDL_CreateWindow("Pendulum", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
	Surface = SDL_GetWindowSurface(Window);
	Pendulums = new DoublePendulum*[PendulumsNumber];
	this->InitPendulums();
}

Simulation::~Simulation(){
  delete Pendulums;
	SDL_FreeSurface(Surface);
	SDL_DestroyWindow(Window);
	SDL_Quit();
}

void Simulation::InitPendulums(){
	double HalfPi = M_PI/2.0;
	for(int i = 0; i < PendulumsNumber; i++){
		Pendulums[i] = new DoublePendulum(WindowWidth, WindowHeight, FPS, Surface, HalfPi);
		HalfPi -= 0.0001;
	}
}
		
void Simulation::ErasePendulums(){
	for(int i = 0; i < PendulumsNumber; i++){
		Pendulums[i]->Erase();
	}
}

void Simulation::AccelerationCorrection(){
	for(int i = 0; i < PendulumsNumber; i++){
		if(Pendulums[i]->IsAccelerated()){
			delete Pendulums[i];
			for(int j = i; j < PendulumsNumber - 1; j++){
				Pendulums[j] = Pendulums[j + 1];
			}
			PendulumsNumber--;
		}
	}
}

void Simulation::UpdatePendulums(){
	for(int i = 0; i < PendulumsNumber; i++){
		Pendulums[i]->Update();
	}
}

void Simulation::DrawPendulums(){
	for(int i = 0; i < PendulumsNumber; i++){
		Pendulums[i]->Draw();
	}
}

void Simulation::Cycle(){
	while (!Quit){
		double IterationStart = SDL_GetPerformanceCounter();
		while(SDL_PollEvent(&Event))
			if(Event.type == SDL_QUIT)
				Quit = true;
				
	 	this->ErasePendulums();
	 	this->AccelerationCorrection();
	 	this->UpdatePendulums();
	 	this->DrawPendulums();
		SDL_UpdateWindowSurface(Window);
		
		double IterationEnd = SDL_GetPerformanceCounter();
		double ElapsedSeconds = (IterationEnd - IterationStart) / (double)SDL_GetPerformanceFrequency();
		double Delay = 33.333 - (ElapsedSeconds * 1000.0);
		if(Delay > 0)
			SDL_Delay(std::max(0, (int) Delay));
	}
}

#pragma once

#include <SDL.h>

#include "DoublePendulum.hpp"

class Simulation{
	public:
		Simulation(int PendulumsNumber);
		~Simulation();
		void InitPendulums();
		void ErasePendulums();
		void AccelerationCorrection();
		void UpdatePendulums();
		void DrawPendulums();
		void Cycle();
	private:
		int WindowWidth, WindowHeight;
		long double FPS;
		SDL_Surface *Surface = nullptr;
		SDL_Window *Window = nullptr;
		bool Quit;
		SDL_Event Event;
		DoublePendulum **Pendulums = nullptr;
		int PendulumsNumber;
};

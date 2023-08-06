#pragma once

#include <SDL.h>
#define _USE_MATH_DEFINES
#include <cmath>

#include "Node.hpp"

class DoublePendulum{
	public:
		DoublePendulum(int WindowWidth, int WindowHeight, long double FPS, SDL_Surface *Surface, long double InitialAngle = M_PI);
		~DoublePendulum();
		void Erase();
		bool IsAccelerated();
		void Update();
		void Draw();
	private:
		Node *FirstNode;
		Node *SecondNode;
		int x, y;
		long double g;
		int WindowWidth;
		SDL_Surface *Surface;
};

#pragma once

#include <SDL.h>

const SDL_Color White = {0xff, 0xff, 0xff};
const SDL_Color LessWhite = {254, 0xff, 0xff};
const SDL_Color Black = {0x00, 0x00, 0x00};

void DrawPixel(int x, int y, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color = White);
bool IsPixelOfGivenColor(int x, int y, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color = LessWhite);
void DrawLine(int x0, int y0, int x1, int y1, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color = White, SDL_Color NotToEraseColor = LessWhite);
void DrawCircle(int x, int y, int Radius, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color = White, SDL_Color NotToEraseColor = LessWhite);

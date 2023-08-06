#include <cmath>

#include "DrawingFunctions.hpp"

void DrawPixel(int x, int y, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color){
	uint8_t* PixelPtr = (uint8_t*)MySurface->pixels + (y * WindowWidth + x) * 4;
	*(PixelPtr + 2) = Color.r;
	*(PixelPtr + 1) = Color.g;
	*(PixelPtr) = Color.b;
}

bool IsPixelOfGivenColor(int x, int y, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color){
    uint8_t* PixelPtr = (uint8_t*)MySurface->pixels + (y * WindowWidth + x) * 4;
    if(*(PixelPtr + 2) == Color.r && *(PixelPtr + 1) == Color.g && *(PixelPtr) == Color.b)
    	return true;
    return false;
}

void DrawLine(int x0, int y0, int x1, int y1, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color, SDL_Color NotToEraseColor){
  double x = x1 - x0;
	double y = y1 - y0;
	double length = sqrt(x * x + y * y);
	double addx = x / length;
	double addy = y / length;
	x = x0;
	y = y0;
	for(int i = 0; i < length; i++){
		if(!IsPixelOfGivenColor((int)x, (int)y, MySurface, WindowWidth, NotToEraseColor))
			DrawPixel((int)x, (int)y, MySurface, WindowWidth, Color);
		x += addx;
		y += addy;
	}
}

void DrawCircle(int x, int y, int Radius, SDL_Surface *MySurface, int WindowWidth, SDL_Color Color, SDL_Color NotToEraseColor){
  for(int w = 0; w < 2 * Radius; w++){
    for(int h = 0; h < 2 * Radius ; h++){
      int dx = Radius - w;
      int dy = Radius - h;
      if((dx * dx + dy * dy) < (Radius * Radius)){
      	if(!IsPixelOfGivenColor(x + dx, y - dy, MySurface, WindowWidth, NotToEraseColor))
      		DrawPixel(x + dx, y - dy, MySurface, WindowWidth, Color);
			}
    }
  }
}

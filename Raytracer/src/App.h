#pragma once
#include "World.h"

#include <SDL.h>

class App {
public:
	App();
	void Loop();
	void Render();
	float TimedRender();
	bool HandleEvent(SDL_Event* e);
	~App();
private:
	World world;
	bool programOpen = false;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
	std::array<uint8_t, WINDOW_WIDTH * WINDOW_HEIGHT * 4> texturePixels;
};
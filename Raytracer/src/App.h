#pragma once
#include "FrameManager.h"

#include <SDL.h>
#include <chrono>

class App {
public:
	App();
	void Loop();
	void PresentRender();
	void SleepForSteadyFPS();
	bool HandleEvent(SDL_Event* e);
	bool HandleMovement();
	~App();
private:
	bool programOpen = false;
	std::chrono::time_point<std::chrono::steady_clock> lastExecution;

	FrameManager frameManager;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
};
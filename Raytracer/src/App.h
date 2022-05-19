#pragma once
#include "FrameManager.h"

#include <SDL.h>
#include <chrono>

// Handles stopping the render threads and rendering to screen.
class RayTracer {
public:
	RayTracer();
	void Loop();
	void PresentRender();
	void SleepForSteadyFPS();
	bool HandleEvent(SDL_Event* e);
	bool HandleMovement();
	~RayTracer();
private:
	bool programOpen = false;
	std::chrono::time_point<std::chrono::steady_clock> lastExecution;

	FrameManager frameManager;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
};
#include "App.h"
#include "World.h"
#include "Constants.h"

#include <iostream>
#include <chrono>
#include <SDL.h>
#include <gtx/rotate_vector.hpp>
#include <thread>


App::App() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		programOpen = false;
		return;
	}

	window = SDL_CreateWindow("RayTracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		programOpen = false;
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	programOpen = true;
	lastExecution = std::chrono::steady_clock::now();

	texture = SDL_CreateTexture(renderer, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

	SDL_SetRelativeMouseMode(SDL_TRUE);
}
App::~App() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void App::Loop() {

	// Render at start
	frameManager.StartNewFrame();

	SDL_Event event;
	while (programOpen) {
		bool needReRendering = false;
		while (SDL_PollEvent(&event))
			needReRendering |= HandleEvent(&event);

		needReRendering |= HandleMovement();
		PresentRender();

		if (needReRendering)
			frameManager.StartNewFrame();

		SleepForSteadyFPS();
	}
}

void App::PresentRender() {

	// If nothing is happening don't bother updating the screen
	if (!frameManager.NeedUpdatingTexture())
		return;

	void* pixels = frameManager.GetTexturePixelsToPresent();

	SDL_Rect srcRect, bounds;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = WINDOW_WIDTH;
	srcRect.h = WINDOW_HEIGHT;
	bounds = srcRect;
	SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * 4);
	SDL_RenderCopy(renderer, texture, &srcRect, &bounds);
	SDL_RenderPresent(renderer);

	frameManager.ContinueWorkingOnImage();
}

void App::SleepForSteadyFPS() {
	auto time = std::chrono::steady_clock::now();
	int deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(time - lastExecution).count();
	std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000.0f / (float)FPS) - deltaTime));
	lastExecution = time;
}

bool App::HandleEvent(SDL_Event* e) {
	if (e->type == SDL_QUIT) {
		programOpen = false;
	}

#if CAN_MOVE_CAMERA
	if (e->type == SDL_MOUSEMOTION) {
		World& world = frameManager.GetWorld();
		world.camera.SetForwardVector(world.camera.GetForwardVector() - MOUSE_SENSITIVITY * ((float)e->motion.xrel * world.camera.GetUDirection() + (float)e->motion.yrel * world.camera.GetVDirection()));
		return true;
	}
#endif

	return false;
}

bool App::HandleMovement() {
	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	World& world = frameManager.GetWorld();
	bool renderAgain = false;

#if CAN_MOVE_CAMERA
	if (keystate[SDL_SCANCODE_D]) {
		glm::vec3 dir = world.camera.GetUDirection();
		dir.y = 0;
		world.camera.pos -= dir * (float)MOVEMENT_SPEED;
		renderAgain = true;
	}
	if (keystate[SDL_SCANCODE_A]) {
		glm::vec3 dir = world.camera.GetUDirection();
		dir.y = 0;
		world.camera.pos += dir * (float)MOVEMENT_SPEED;
		renderAgain = true;
	}
	if (keystate[SDL_SCANCODE_W]) {
		glm::vec3 dir = world.camera.GetForwardVector();
		dir.y = 0;
		world.camera.pos += dir * (float)MOVEMENT_SPEED;
		renderAgain = true;
	}
	if (keystate[SDL_SCANCODE_S]) {
		glm::vec3 dir = world.camera.GetForwardVector();
		dir.y = 0;
		world.camera.pos -= dir * (float)MOVEMENT_SPEED;
		renderAgain = true;
	}
#endif

	return renderAgain;
}

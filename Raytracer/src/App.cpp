#include "App.h"
#include "World.h"
#include "Constants.h"

#include <iostream>
#include <chrono>
#include <SDL.h>
#include <gtx/rotate_vector.hpp>


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

	texture = SDL_CreateTexture(renderer, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
}
App::~App() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void App::Loop() {
	// Render at start
	world.camera.pos = glm::rotateY(world.camera.pos, 0.5f);
	world.camera.forwardVector = glm::rotateY(world.camera.forwardVector, 0.5f);
	world.camera.upVector = glm::rotateY(world.camera.upVector, 0.5f);
	TimedRender();

	SDL_Event event;
	while (programOpen) {
		while (SDL_WaitEvent(&event)) {

			bool needRendering = HandleEvent(&event);
			if (!needRendering)
				break;
			
			/* Do many timed renders and take the average */
			const int renderAmount = 20;
			float total = 0;
			for (int i = 0; i < renderAmount; i++) {
				world.camera.pos = glm::rotateY(world.camera.pos, 0.05f);
				world.camera.forwardVector = glm::rotateY(world.camera.forwardVector, 0.05f);
				world.camera.upVector = glm::rotateY(world.camera.upVector, 0.05f);

				total += TimedRender();
			}
			std::cout << "On average it took " << total / renderAmount << " seconds";
		}
	}
}

void App::Render() {
	int index = 0;
	for (int y = 0; y < WINDOW_HEIGHT; y++) {
		for (int x = 0; x < WINDOW_WIDTH; x++) {
			glm::u8vec3 color = world.CalculateColorForScreenPosition(x, y);
			texturePixels[index]   = 255;
			texturePixels[index+1] = color.b;
			texturePixels[index+2] = color.g;
			texturePixels[index+3] = color.r;
			index += 4;
		}
	}
	int error = SDL_UpdateTexture(texture, NULL, texturePixels.data(), WINDOW_WIDTH * 4 * sizeof(uint8_t));
	if (error != 0)
		std::cout << SDL_GetError() << std::endl;
	else std::cout << "rendered";

	SDL_Rect srcRect, bounds;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = WINDOW_WIDTH;
	srcRect.h = WINDOW_HEIGHT;
	bounds = srcRect;
	SDL_RenderCopy(renderer, texture, &srcRect, &bounds);
	SDL_RenderPresent(renderer);
}

float App::TimedRender() {
	auto startTime = std::chrono::system_clock::now();
	Render();
	auto endTime = std::chrono::system_clock::now();
	float time = (float)std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000;
	std::cout << "Rendering took " << time << " seconds." << std::endl;
	return time;
}

bool App::HandleEvent(SDL_Event* e) {
	if (e->type == SDL_QUIT) {
		programOpen = false;
		return false;
	}
	if (e->type == SDL_KEYDOWN) {
		return true;
	}

	return false;
}

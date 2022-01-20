#pragma once

#include "World.h"

#include <thread>
#include <atomic>

enum class ThreadState {
	Sleep, Quit, Work, AllComplete
};

class FrameManager {
public:
	void StartNewFrame();

	bool NeedUpdatingTexture();
	// Waits for all the threads to join and returns texturePixels
	void* GetTexturePixelsToPresent();
	void ContinueWorkingOnImage();

	World& GetWorld() { return world; }
private:
	void ThreadWork(uint32_t index);
	
	World world;
	std::vector<std::thread> threads;
	std::atomic<ThreadState> threadState = ThreadState::Work;
	std::atomic<int> runningThreadAmount = 0;
	std::atomic<int> workingThreadAmount = 0;
	std::array<uint8_t, WINDOW_WIDTH * WINDOW_HEIGHT * 4> texturePixels;
};
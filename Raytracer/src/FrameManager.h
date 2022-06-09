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
	void TerminateAllThreads();

	bool NeedUpdatingTexture();
	void FinishFrame();

	// Waits for all the threads to join and returns texturePixels
	void* GetTexturePixelsToPresent();
	void ContinueWorkingOnImage();

	World& GetWorld() {	return world; } 
private:
	void ThreadWork(uint32_t index);

	float time;
	World world;
	std::chrono::system_clock::time_point frameStartTime; // Benchmarking

	std::vector<std::thread> threads;
	std::atomic<ThreadState> threadState = ThreadState::Work;
	std::atomic<int> runningThreadAmount = 0;
	std::atomic<int> workingThreadAmount = 0;
	std::array<uint8_t, WINDOW_WIDTH * WINDOW_HEIGHT * 4> texturePixels;
};
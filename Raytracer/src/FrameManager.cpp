#include "FrameManager.h"
#include "Constants.h"
#include <chrono>
#include <sstream>
#include <cstring>

void FrameManager::StartNewFrame() {
	TerminateAllThreads();

	memset(&texturePixels, 0, texturePixels.size());
	threadState = ThreadState::Work;

	try {
		threads.reserve(THREAD_COUNT);
		for (uint32_t i = 0; i < THREAD_COUNT; i++) {
			runningThreadAmount++;
			threads.push_back(std::thread(&FrameManager::ThreadWork, this, i));
		}
	}
	catch (...) {
		throw;
	}

#if LOG_BENCHMARK
	frameStartTime = std::chrono::system_clock::now();
#endif
}
void FrameManager::TerminateAllThreads() {
	threadState = ThreadState::Quit;
	while (runningThreadAmount != 0)
		std::this_thread::sleep_for(std::chrono::microseconds(5));
	for (int i = 0; i < threads.size(); i++)
		threads[i].join();
	threads.clear();
}

bool FrameManager::NeedUpdatingTexture() {
	if (threadState == ThreadState::AllComplete)
		return false;

	if (runningThreadAmount == 0)
		FinishFrame();

	return true;
}

void FrameManager::FinishFrame() {
	threadState = ThreadState::AllComplete;

#if LOG_BENCHMARK
	auto endTime = std::chrono::system_clock::now();
	std::chrono::duration<double> dur = endTime - frameStartTime;
	std::cout << "Frame calculating time: " << dur.count() << std::endl;
#endif
}

void* FrameManager::GetTexturePixelsToPresent() {
	if (threadState == ThreadState::AllComplete)
		return texturePixels.data();

	// Wait for all the threads to stop calculating
	threadState = ThreadState::Sleep;
	while (workingThreadAmount != 0) {
		std::this_thread::sleep_for(std::chrono::microseconds(5));
	}

	return texturePixels.data();
}

void FrameManager::ContinueWorkingOnImage() {
	if (threadState != ThreadState::AllComplete)
		threadState = ThreadState::Work;
}

void FrameManager::ThreadWork(uint32_t threadIndex) {
	workingThreadAmount++;
	const uint32_t pixelAmount = (uint32_t)texturePixels.size() / 4;
	const uint32_t startIndex = (threadIndex * pixelAmount) / THREAD_COUNT * 4;
	const uint32_t endIndex = ((threadIndex+1) * pixelAmount) / THREAD_COUNT * 4;

	uint32_t currentPixelIndex = startIndex;
	uint32_t nextSpreadStartOffset = 1;

	while (true) {
		while (threadState == ThreadState::Work) {
			glm::vec3 col = world.CalculateColorForScreenPosition((currentPixelIndex / 4) % WINDOW_WIDTH, (currentPixelIndex / 4) / WINDOW_WIDTH);
			int length = (int)texturePixels.size();
			texturePixels[currentPixelIndex + 0] = 255;
			texturePixels[currentPixelIndex + 1] = (uint8_t)col.b;
			texturePixels[currentPixelIndex + 2] = (uint8_t)col.g;
			texturePixels[currentPixelIndex + 3] = (uint8_t)col.r;
			currentPixelIndex += 4 * PIXEL_CALCULATING_OREDER_SPREAD;

			if (currentPixelIndex >= endIndex) {
				currentPixelIndex = startIndex + nextSpreadStartOffset * 4;
				nextSpreadStartOffset++;
			}

			// Quit if this thread is complete
			if (nextSpreadStartOffset > PIXEL_CALCULATING_OREDER_SPREAD) {
				workingThreadAmount--;
				runningThreadAmount--;
				return;
			}
		}

		if (threadState == ThreadState::Quit) {
			workingThreadAmount--;
			runningThreadAmount--;
			return;
		}

		// Wait until thread needs to be used again
		workingThreadAmount--;
		while (threadState == ThreadState::Sleep)
			std::this_thread::sleep_for(std::chrono::microseconds(5));
		workingThreadAmount++;
	}
}

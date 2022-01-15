#include "FrameManager.h"

void FrameManager::StartNewFrame() {
	std::cout << "Starting new frame" << std::endl;

	// Wait for all the threads to stop calculating
	threadState = ThreadState::Quit;
	while (workingThreadAmount != 0)
		std::this_thread::sleep_for(std::chrono::microseconds(5));

	memset(&texturePixels, 0, texturePixels.size());
	threadState = ThreadState::Work;

	try {
		threads.reserve(EXTRA_THREAD_COUNT);
		for (uint32_t i = 0; i < EXTRA_THREAD_COUNT; i++) {
			runningThreadAmount++;
			threads.push_back(std::thread(&FrameManager::ThreadWork, this, i));
		}
	}
	catch (...) {
		throw;
	}

}

bool FrameManager::NeedUpdatingTexture() {
	if (threadState == ThreadState::AllComplete)
		return false;

	if (runningThreadAmount == 0)
		threadState = ThreadState::AllComplete;

	return true;
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
	const uint32_t startIndex = (threadIndex * texturePixels.size()) / EXTRA_THREAD_COUNT;
	const uint32_t endIndex = ((threadIndex+1) * texturePixels.size()) / EXTRA_THREAD_COUNT;
	uint32_t currentPixelIndex = startIndex;

	while (true) {
		while (threadState == ThreadState::Work) {
			glm::vec3 col = world.CalculateColorForScreenPosition((currentPixelIndex / 4) % WINDOW_WIDTH, (currentPixelIndex / 4) / WINDOW_WIDTH);
			texturePixels[currentPixelIndex] = 255;
			texturePixels[currentPixelIndex + 1] = col.b;
			texturePixels[currentPixelIndex + 2] = col.g;
			texturePixels[currentPixelIndex + 3] = col.r;
			currentPixelIndex += 4;

			// Quit if this thread is complete
			if (currentPixelIndex == endIndex) {
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

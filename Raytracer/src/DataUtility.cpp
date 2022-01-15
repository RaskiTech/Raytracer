#include "DataUtility.h"

#include "stb_image/stb_image.h"
#include <glm.hpp>
#include <string>
#include <iostream>

void Camera::SetForwardVector(const glm::vec3& lookVector) {
	forwardVector = glm::normalize(lookVector);
	
	uVec = glm::cross(globalCameraUpVector, forwardVector);
	vVec = glm::cross(forwardVector, uVec);
}

Skybox::Skybox(const std::string& filepath) {
	int x, y, channels;
	skyboxImageData = (char*)stbi_load(filepath.c_str(), &x, &y, &channels, 3);
	size.x = x;
	size.y = y;
	if (channels != 3) {
		std::cout << "The skybox had " << channels << " channels instead of 3." << std::endl;
		__debugbreak();
	}
}

Skybox::~Skybox() {
	stbi_image_free(skyboxImageData);
}

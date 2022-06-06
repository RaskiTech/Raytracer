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
	stbi_set_flip_vertically_on_load(true);

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

// Can do some optimizing https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool BoundingBox::DoesRayHit(const Ray& r) const {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	if (r.direction.x >= 0) {
		tmin = (minCoord.x - r.pos.x) / r.direction.x;
		tmax = (maxCoord.x - r.pos.x) / r.direction.x;
	}
	else {
		tmin = (maxCoord.x - r.pos.x) / r.direction.x;
		tmax = (minCoord.x - r.pos.x) / r.direction.x;
	}

	if (r.direction.y >= 0) {
		tymin = (minCoord.y - r.pos.y) / r.direction.y;
		tymax = (maxCoord.y - r.pos.y) / r.direction.y;
	}
	else {
		tymin = (maxCoord.y - r.pos.y) / r.direction.y;
		tymax = (minCoord.y - r.pos.y) / r.direction.y;
	}

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	tmin = glm::max(tmin, tymin);
	tmax = glm::min(tmax, tymax);

	if (r.direction.z >= 0) {
		tzmin = (minCoord.z - r.pos.z) / r.direction.z;
		tzmax = (maxCoord.z - r.pos.z) / r.direction.z;
	}
	else {
		tzmin = (maxCoord.z - r.pos.z) / r.direction.z;
		tzmax = (minCoord.z - r.pos.z) / r.direction.z;
	}

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	tmin = glm::max(tmin, tzmin);
	tmax = glm::min(tmax, tzmax);

	// Filter out collisions where the whole object is behind
	if (tmin < 0.0f && tmax < 0.0f)
		return false;

	return true;
}

glm::vec3 CheckeredTexture::GetColorValue(const glm::vec2& uv, const glm::vec3& p) const {
	const float scale = 5.0f;

	float x = p.x;
	float y = p.y;
	float z = p.z;
	
	float sines = glm::sin(scale * x) * glm::sin(scale * y) * glm::sin(scale * z);
	if (sines < 0.0f)
		return color1;
	else
		return color2;
}

ImageTexture::ImageTexture(const std::string& path) {
	stbi_set_flip_vertically_on_load(true);

	int x, y, channels;
	imageData = (char*)stbi_load(path.c_str(), &x, &y, &channels, STBI_rgb);
	size.x = x;
	size.y = y;
	if (channels != 3) {
		std::cout << "The skybox had " << channels << " channels instead of 3. Does the path exists?" << std::endl;
		__debugbreak();
	}
}
ImageTexture::~ImageTexture() {
	stbi_image_free(imageData);
}
glm::vec3 ImageTexture::GetColorValue(const glm::vec2& uv, const glm::vec3& p) const {
	int index = 3 * (int)((int)(uv.y * size.y) * size.x + (int)(uv.x * size.x));

	uint8_t c1 = imageData[index];
	uint8_t c2 = imageData[index + 1];
	uint8_t c3 = imageData[index + 2];
	return glm::vec3{ (float)c1, (float)c2, (float)c3 } / 255.0f;
}

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

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

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

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	// Filter out collision behind the box
	if (tmin < 0.0f && tmax < 0.0f)
		return false;

	return true;
}


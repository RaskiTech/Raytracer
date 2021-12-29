#include "World.h"
#include "Constants.h"

#include <iostream>
#include <array>
#include <glm.hpp>
#include <gtx/compatibility.hpp>
#include "stb_image/stb_image.h"

#include <cstdlib>
static float Random01() { return ((float)rand() / (float)RAND_MAX); }
static glm::vec3 GetRandomUnitSpherePoint() {
	while (true) {
		glm::vec3 sample = { Random01(), Random01(), Random01() };
		if (sample.x + sample.y + sample.z < 1.0f)
			return sample - 0.5f;
	}
}

World::World() : 
	objects({
		Object(glm::vec3{0, 0, 0},  glm::vec3{0, 0, 0}, 1, ObjectType::PlaneY, Material({ 0.8f, 0.8f, 0.8f }, 0.0f)),

		Object(glm::vec3{3, 3, 0}, glm::vec3{0, 0, 0}, 3, ObjectType::Sphere, Material({ 0.2f, 0.3f, 0.8f }, 0.0f)),
		Object(glm::vec3{-4, 2, 0}, glm::vec3{0, 0, 0}, 2, ObjectType::Sphere, Material({ 0.2f, 0.8f, 0.3f }, 1.0f)),
		Object(glm::vec3{1, 2, 6}, glm::vec3{0, 0, 0}, 2, ObjectType::Sphere, Material({ 0.9f, 0.9f, 0.9f }, 0.1f)),
		Object(glm::vec3{-5, 0.75f, 3}, glm::vec3{0, 0, 0}, 0.75f, ObjectType::Sphere, Material({ 0.2f, 0.8f, 0.3f }, 0.3f)),

		Object(glm::vec3{-3, 1, -6}, glm::vec3{0, 0, 0}, 1, ObjectType::Sphere, Material({ 0.9f, 0.9f, 0.9f }, 0.9f)),
		Object(glm::vec3{17, 5, 8}, glm::vec3{0, 0, 0}, 5, ObjectType::Sphere, Material({ 0.2f, 0.8f, 0.3f }, 0.0f)),
		Object(glm::vec3{0, 2, -20}, glm::vec3{0, 0, 0}, 2, ObjectType::Sphere, Material({ 0.9f, 0.9f, 0.9f }, 0.0f)),
	}) 
{
	camera.forwardVector = glm::normalize(camera.forwardVector);
	camera.upVector = glm::normalize(camera.upVector);
	lightVector = glm::normalize(lightVector);

	int x, y, channels;
	skybox.skyboxImageData = (char*)stbi_load("src/stb_image/skybox12.png", &x, &y, &channels, 3);
	skybox.size.x = (float)x;
	skybox.size.y = (float)y;
	if (channels != 3) {
		std::cout << "The skybox had " << channels << " channels instead of 3." << std::endl;
		__debugbreak;
	}
}
World::~World() {
	stbi_image_free(skybox.skyboxImageData);
}

glm::u8vec3 World::CalculateColorForScreenPosition(int x, int y) {
	Ray ray;
	glm::vec2 onePixelOffset = { -(1.0f / WINDOW_WIDTH) * ((float)WINDOW_WIDTH / WINDOW_HEIGHT) * FIELD_OF_VIEW, -(1.0f / WINDOW_HEIGHT) * FIELD_OF_VIEW };
	glm::vec2 offset = { -((float)x / WINDOW_WIDTH - 0.5f) * ((float)WINDOW_WIDTH/WINDOW_HEIGHT) * FIELD_OF_VIEW, -((float)y / WINDOW_HEIGHT - 0.5f) * FIELD_OF_VIEW };
	ray.pos = camera.pos;
	glm::vec3 color{ 0 };

	for (int x = 0; x < SAMPLES_PER_PIXEL_AXIS; x++) {
		for (int y = 0; y < SAMPLES_PER_PIXEL_AXIS; y++) {
			ray.direction = camera.forwardVector;
			ray.direction += (offset.x + onePixelOffset.x * ((float)x / SAMPLES_PER_PIXEL_AXIS)) * glm::cross(camera.upVector, camera.forwardVector)
				+ camera.upVector * (offset.y + onePixelOffset.y * ((float)y / SAMPLES_PER_PIXEL_AXIS));
			ray.direction = glm::normalize(ray.direction);

			color += GetRayColor(ray, LIGHT_BOUNCE_AMOUNT);
		}
	}
	color /= SAMPLES_PER_PIXEL_AXIS * SAMPLES_PER_PIXEL_AXIS;

	// Gamma color correct for diffuse materials
	color = glm::sqrt(color);

	glm::u8vec3 color8Bit = { (char)(color.r * 256), (char)(color.g * 256), (char)(color.b * 256) };
	return color8Bit;
}

glm::vec3 World::GetRayColor(const Ray& ray, int bounceAmount) {
	glm::vec3 hitPoint;
	glm::vec3 hitNormal;
	int obj = -1;
	float distance;
	{
		glm::vec3 thisHitPoint;
		glm::vec3 thisHitNormal;
		float minDistanceHit = std::numeric_limits<float>::max();
		for (int i = 0; i < objects.size(); i++) {
			if (!Intersect(i, ray, thisHitPoint, thisHitNormal, distance))
				continue;

			if (distance < minDistanceHit) {
				obj = i;
				hitPoint = thisHitPoint;
				hitNormal = thisHitNormal;
				minDistanceHit = distance;
			}
		}
	}

	if (obj == -1)
		return GetSkyboxPixel(ray.direction);

	Ray reflectedRay;

	/* Diffuse */
	if (objects[obj].material.reflectiveness == 0.0f) {
		const float lightDarknerFactor = 0.5f;

		if (bounceAmount == 0)
			return glm::vec3{ 0 };

		reflectedRay.direction = glm::normalize(hitNormal + 2.0f * GetRandomUnitSpherePoint());
		reflectedRay.pos = hitPoint;

		glm::vec3 pixelColor = lightDarknerFactor * GetRayColor(reflectedRay, bounceAmount - 1) * objects[obj].material.color;

		return pixelColor;
	}
	/* Metal */
	else {
		float shadowMultiplier = 1.0f - (1.0f - glm::clamp(glm::dot(hitNormal, lightVector) + 1.0f, 0.0f, 1.0f)) * SELF_SHADOW_INTENSITY;
		if (bounceAmount == 0 || objects[obj].material.reflectiveness == 0)
			return objects[obj].material.color * shadowMultiplier;

		reflectedRay.pos = hitPoint;
		reflectedRay.direction = ray.direction - 2.0f * hitNormal * glm::dot(ray.direction, hitNormal);

		glm::vec3 pixelColor = (1.0f - objects[obj].material.reflectiveness) * objects[obj].material.color
			+ objects[obj].material.reflectiveness * GetRayColor(reflectedRay, bounceAmount - 1);
		return pixelColor * shadowMultiplier;
	}
}

bool World::Intersect(const int& objectIndex, const Ray& ray, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance) {
	switch (objects[objectIndex].type) {
		case ObjectType::Plane: {
			return false;
		}
		case ObjectType::PlaneY: {
			float t = -ray.pos.y / ray.direction.y;
			if (t < 1e-3)
				return false;

			outDistance = t;
			outHitPoint = ray.pos + t * ray.direction;
			outHitNormal = { 0.0f, 1.0f, 0.0f };
			return true;
		}
		case ObjectType::Sphere: {
			glm::vec3 distance = ray.pos - objects[objectIndex].pos;
			float p1 = -glm::dot(ray.direction, distance);
			float p2sqr = p1 * p1 - glm::dot(distance, distance) + objects[objectIndex].scale * objects[objectIndex].scale;
			if (p2sqr < 0)
				return false;
			outDistance = p1 - sqrt(p2sqr);
			if (outDistance < 0)
				return false;
			outHitPoint = ray.pos + outDistance * ray.direction;
			outHitNormal = glm::normalize(outHitPoint - objects[objectIndex].pos);
			return true;
		}
		case ObjectType::Cube: {
			/*
			float tmin, tmax, tymin, tymax, tzmin, tzmax;
			

			tmin = (bounds[r.sign[0]].x - r.orig.x) * r.invdir.x;
			tmax = (bounds[1 - r.sign[0]].x - r.orig.x) * r.invdir.x;
			tymin = (bounds[r.sign[1]].y - r.orig.y) * r.invdir.y;
			tymax = (bounds[1 - r.sign[1]].y - r.orig.y) * r.invdir.y;

			if ((tmin > tymax) || (tymin > tmax))
				return false;
			if (tymin > tmin)
				tmin = tymin;
			if (tymax < tmax)
				tmax = tymax;

			tzmin = (bounds[r.sign[2]].z - r.orig.z) * r.invdir.z;
			tzmax = (bounds[1 - r.sign[2]].z - r.orig.z) * r.invdir.z;

			if ((tmin > tzmax) || (tzmin > tmax))
				return false;
			if (tzmin > tmin)
				tmin = tzmin;
			if (tzmax < tmax)
				tmax = tzmax;

			return true;
			*/
			return false;
		}
		default:
			return false;
	}
}


glm::vec3 World::GetSkyboxPixel(const glm::vec3& direction) {
	glm::vec2 pixel = glm::vec2{ (0.5f + glm::atan(direction.x, direction.z) / (glm::two_pi<float>())) * skybox.size.x, (0.5f - glm::asin(direction.y) / glm::pi<float>()) * skybox.size.y };
	int intPixelX = glm::floor(pixel.x);
	int intPixelY = glm::floor(pixel.y);

	float Xfraction = (intPixelX + 0.5f - pixel.x);
	float Yfraction = (intPixelY + 0.5f - pixel.y);
	float absX = glm::abs(Xfraction);
	float absY = glm::abs(Yfraction);

	float originalMultiplier = (1.0f - absX) * (1.0f - absY);
	float otherXMultiplier = absX * (1.0f - absY);
	float otherYMultiplier = (1.0f - absX) * absY;
	float otherXYMultiplier = absX * absY;

	int otherX = Xfraction < 0 ? (intPixelX + 1) : (intPixelX - 1);
	int otherY = Yfraction < 0 ? (intPixelY + 1) : (intPixelY - 1);
	otherX = otherX < 0 ? otherX + skybox.size.x : otherX % skybox.size.x;
	otherY = otherY < 0 ? otherY + skybox.size.x : otherY % skybox.size.y;

	glm::vec3 col = GetSkyboxPixel(pixel.x, pixel.y) * originalMultiplier;
	col += GetSkyboxPixel(otherX, pixel.y) * otherXMultiplier;
	col += GetSkyboxPixel(pixel.x, otherY) * otherYMultiplier;
	col += GetSkyboxPixel(otherX, otherY) * otherXYMultiplier;


	return col;
}
glm::vec3 World::GetSkyboxPixel(int x, int y) {
	int index = 3 * (int)(y * skybox.size.x + x);
	uint8_t c1 = skybox.skyboxImageData[index];
	uint8_t c2 = skybox.skyboxImageData[index + 1];
	uint8_t c3 = skybox.skyboxImageData[index + 2];

	return glm::vec3{ (float)*(uint8_t*)&skybox.skyboxImageData[index], (float)*(uint8_t*)&skybox.skyboxImageData[index + 1], (float)*(uint8_t*)&skybox.skyboxImageData[index + 2] } / 255.0f;
}
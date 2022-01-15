#pragma once
#include "Constants.h"
#include "DataUtility.h"

#include <iostream>
#include <array>
#include <vector>
#include <glm.hpp>


class World {
public:
	World();

	glm::u8vec3 CalculateColorForScreenPosition(int x, int y);
	glm::vec3 GetRayColor(const Ray& ray, int bounceAmount = 0);
	bool Intersect(const int& objectIndex, const Ray& ray, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance);

	glm::vec3 GetSkyboxPixel(const glm::vec3& direction);
	inline glm::vec3 GetSkyboxPixel(int x, int y);

public:
	Skybox skybox;
	Camera camera;
	glm::vec3 lightVector = { 0.0f, 0.707f, -0.707f };
	std::vector<Object> objects;
};
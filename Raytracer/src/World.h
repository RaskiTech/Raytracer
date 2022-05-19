#pragma once
#include "Constants.h"
#include "DataUtility.h"
#include "Object.h"

#include <iostream>
#include <array>
#include <vector>
#include <glm.hpp>


class World {
public:
	World();
	~World();

	glm::u8vec3 CalculateColorForScreenPosition(int x, int y);
	glm::vec3 GetRayColor(const Ray& ray, int bounceAmount = 0);
	Camera& GetWorldCamera() { return camera; }

private:
	glm::vec3 GetSkyboxPixel(const glm::vec3& direction);
	inline glm::vec3 GetSkyboxPixel(int x, int y);

private:
	std::vector<Object*> noBoundingBoxObjects;
	BVH_Node rootNode;

	Skybox skybox;
	Camera camera;
	glm::vec3 lightVector = { 0.0f, 0.707f, -0.707f };
};
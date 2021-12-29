#pragma once
#include "Constants.h"

#include <iostream>
#include <array>
#include <glm.hpp>


enum class ObjectType {
	None, Plane/* In plane rotation is the normal vector*/, PlaneY, Sphere, Cube
};
struct Material {
	Material() = default;
	Material(glm::vec3 color, float reflectiveness = 0.1f) : reflectiveness(reflectiveness), color(color) {}

	float reflectiveness = 0.1f;
	glm::vec3 color = { 1, 1, 1 };
};
struct Object {
	Object(glm::vec3 pos, glm::vec3 rotation, float scale, ObjectType type, Material mat)
		: pos(pos), rotation(rotation), scale(scale), type(type), material(mat) {}
	Object() = default;

	glm::vec3 pos{ 0 };
	glm::vec3 rotation{ 0 };
	float scale = 0;
	ObjectType type = ObjectType::None;
	Material material = Material();
};

struct Ray {
	glm::vec3 pos{ 0 };
	glm::vec3 direction{ 0 };
};

struct Camera {
	glm::vec3 pos = {-10, 3, 0};
	glm::vec3 forwardVector = { 3, -0.5f, 0 };
	glm::vec3 upVector = { 0.5f, 3, 0 };
};
struct Skybox {
	glm::uvec2 size;
	char* skyboxImageData = nullptr;
};
enum class ColorCorrection {
	None, GammaCorrection
};

class World {
public:
	World();
	~World();

	glm::u8vec3 CalculateColorForScreenPosition(int x, int y);
	glm::vec3 GetRayColor(const Ray& ray, int bounceAmount = 0);
	bool Intersect(const int& objectIndex, const Ray& ray, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance);

	glm::vec3 GetSkyboxPixel(const glm::vec3& direction);
	inline glm::vec3 GetSkyboxPixel(int x, int y);

public:
	Skybox skybox;
	Camera camera;
	glm::vec3 lightVector = { 0.0f, 0.707f, -0.707f };
	std::array<Object, WORLD_OBJECT_COUNT> objects;
};
#pragma once

#include <glm.hpp>
#include <string>

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
	Camera(glm::vec3 pos, const glm::vec3& forwardVector) : pos(pos) { SetForwardVector(forwardVector); }
	Camera() : pos({ 0, 0, 0 }) { SetForwardVector({ 1, 0, 0 }); }

	void SetForwardVector(const glm::vec3& lookVector);
	const glm::vec3& GetForwardVector() { return forwardVector; };
	const glm::vec3& GetUDirection() { return uVec; }
	const glm::vec3& GetVDirection() { return vVec; }

	glm::vec3 pos;

private:
	glm::vec3 forwardVector;
	glm::vec3 uVec;
	glm::vec3 vVec;

	// Normalized camera up direction
	const glm::vec3 globalCameraUpVector = { 0, 1, 0 };
};
struct Skybox {
	Skybox(const std::string& filepath);
	~Skybox();

	glm::uvec2 size;
	char* skyboxImageData = nullptr;
};


enum class ColorCorrection {
	None, GammaCorrection
};
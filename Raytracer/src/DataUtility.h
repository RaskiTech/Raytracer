#pragma once

#include <glm.hpp>
#include <string>


#include <cstdlib>
static float Random01() { return ((float)rand() / (float)RAND_MAX); }
static glm::vec3 GetRandomUnitSpherePoint() {
	while (true) {
		glm::vec3 sample = { Random01(), Random01(), Random01() };
		if (sample.x + sample.y + sample.z < 1.0f)
			return sample - 0.5f;
	}
}
static glm::vec2 GetRandomUnitCirclePoint() {
	while (true) {
		glm::vec2 sample = { Random01(), Random01() };
		if (sample.x + sample.y < 1.0f)
			return sample - 0.5f;
	}
}

struct Ray {
	glm::vec3 pos{ 0 };
	glm::vec3 direction{ 0 };
};

enum class MaterialType {
	None, Diffuse, Metal
};
struct Material {
	Material() = default;
	static Material CreateDiffuse(glm::vec3 color) { return Material{ MaterialType::Diffuse, 0.0f, color }; }
	static Material CreateMetal(glm::vec3 color, float reflectiveness) { return Material{ MaterialType::Metal, reflectiveness, color }; }

	MaterialType materialType = MaterialType::None;
	float reflectiveness = 0.0f;
	glm::vec3 color = { 1, 1, 1 };
};

// Axis aligned boundeing box
struct BoundingBox {
	BoundingBox() = default;
	BoundingBox(glm::vec3 minCoord, glm::vec3 maxCoord) : minCoord(minCoord), maxCoord(maxCoord) {}
	bool DoesRayHit(const Ray& ray) const;

	glm::vec3 minCoord{ 0 };
	glm::vec3 maxCoord{ 0 };
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
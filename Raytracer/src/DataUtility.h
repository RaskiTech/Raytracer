#pragma once

#include <glm.hpp>
#include <string>
#include <memory>
#include <iostream>

#if 1
#include <cstdlib>
static float Random01() { return (float)std::rand() / RAND_MAX; }
static float RandomUnitDistance() { return ((float)(std::rand() - RAND_MAX / 2) / (RAND_MAX / 2)); }
#else
#include <random>
#include <chrono>
static float Random01() {
	static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	return distribution(rng);
}
static float RandomUnitDistance() {
	static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	return distribution(rng);
}
#endif

static glm::vec3 GetRandomUnitSpherePoint() {
	while (true) {
		glm::vec3 sample = glm::vec3{ RandomUnitDistance(), RandomUnitDistance(), RandomUnitDistance() };
		if (sample.x * sample.x + sample.y * sample.y + sample.z * sample.z < 1.0f)
			return sample;
	}
}
static glm::vec2 GetRandomUnitCirclePoint() {
	while (true) {
		glm::vec2 sample = { RandomUnitDistance(), RandomUnitDistance() };
		if (sample.x * sample.x + sample.y * sample.y < 1.0f)
			return sample;
	}
}

struct Ray {
	glm::vec3 pos{ 0 };
	glm::vec3 direction{ 0 };
};

struct Object;
struct HitInfo {
	glm::vec3 point;
	glm::vec3 normal;
	Object* object = nullptr;
	float distance = std::numeric_limits<float>::max();
	glm::vec2 uv;
};

struct ColorTexture;
struct CheckeredTexture;
struct ImageTexture;
struct UVTexture;
struct Texture {
	virtual ~Texture() {}
	virtual glm::vec3 GetColorValue(const glm::vec2& uv, const glm::vec3& p) const = 0;

	static std::shared_ptr<UVTexture> CreateUV() { return std::make_shared<UVTexture>(); }
	static std::shared_ptr<ColorTexture> CreateColored(const glm::vec3& col) { return std::make_shared<ColorTexture>(col); }
	static std::shared_ptr<CheckeredTexture> CreateCheckered(const glm::vec3& col1, const glm::vec3& col2) { return std::make_shared<CheckeredTexture>(col1, col2); }
	static std::shared_ptr<ImageTexture> CreateFromImage(const std::string& imagePath) { return std::make_shared<ImageTexture>(imagePath); }
	
};
struct ImageTexture : public Texture {
	ImageTexture(const std::string& path);
	~ImageTexture();

	glm::vec3 GetColorValue(const glm::vec2& uv, const glm::vec3& p) const;

	glm::uvec2 size;
	char* imageData;
};
struct ColorTexture : public Texture {
	ColorTexture() = default;
	ColorTexture(const glm::vec3 color) : color(color) {}

	glm::vec3 GetColorValue(const glm::vec2& uv, const glm::vec3& p) const { return color; }

	glm::vec3 color{ 0 };
};
struct CheckeredTexture : public Texture {
	CheckeredTexture() = default;
	CheckeredTexture(const glm::vec3 color1, const glm::vec3 color2) : color1(color1), color2(color2) {}

	glm::vec3 GetColorValue(const glm::vec2& uv, const glm::vec3& p) const override;

	glm::vec3 color1 { 0.0f };
	glm::vec3 color2 { 1.0f };
};
struct UVTexture : public Texture {
	UVTexture() = default;
	glm::vec3 GetColorValue(const glm::vec2& uv, const glm::vec3& p) const override { return { uv.x, uv.y, 0 }; }
};

enum class MaterialType {
	None, Diffuse, Metal, DiffuseLight
};
struct Material {
	Material() = default;
	static Material CreateDiffuse(std::shared_ptr<Texture> tex) { return Material{ MaterialType::Diffuse, 0.0f, tex }; }
	static Material CreateMetal(float reflectiveness, std::shared_ptr<Texture> tex) { return Material{ MaterialType::Metal, reflectiveness, tex }; }
	static Material CreateDiffuseLight(glm::vec3 emittingColor) { return Material{ MaterialType::DiffuseLight, 0.0f, nullptr, emittingColor }; }

	MaterialType materialType = MaterialType::None;
	float reflectiveness = 0.0f;
	std::shared_ptr<Texture> texture;
	glm::vec3 emittingColor = glm::vec3{ 0 }; // Could also be a texture at some point
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
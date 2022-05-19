#pragma once
#include "DataUtility.h"
#include <vector>

struct Object {
	Object(Material mat) : material(mat) {}
	Object() = default;
	virtual ~Object() = default;

	virtual bool Intersect(const Ray& ray, Object*& outObj, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance) const = 0;
	
	// Return: Has bounding box
	virtual bool GetBoundingBox(BoundingBox& outBox) const = 0;

	Material material;
};

struct Sphere : public Object {
	Sphere(glm::vec3 pos, float radius, Material mat) : Object(mat), pos(pos), radius(radius) {}

	bool Intersect(const Ray& ray, Object*& outObj, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	glm::vec3 pos{ 0 };
	float radius = 0;
};

struct AxisAlignedCube : public Object {
	AxisAlignedCube(glm::vec3 pos, float radius, Material mat) : Object(mat), pos(pos), radius(radius) {}

	bool Intersect(const Ray& ray, Object*& outObj, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	glm::vec3 pos{ 0 };
	float radius = 0;
};

struct YPlane : public Object {
	YPlane(float yPos, Material mat) : Object(mat), yPos(yPos) {}

	bool Intersect(const Ray& ray, Object*& outObj, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	float yPos = 0;
};

struct BVH_Node : public Object {
	BVH_Node(const std::vector<Object*>& scrObjects) : BVH_Node(scrObjects, 0, scrObjects.size()) {}
	BVH_Node(const std::vector<Object*>& scrObjects, int start, int end);
	~BVH_Node();

	bool Intersect(const Ray& ray, Object*& outObj, glm::vec3& outHitPoint, glm::vec3& outHitNormal, float& outDistance) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { outBox = boundingBox; return true; }

	Object* left = nullptr;
	Object* right = nullptr;
	BoundingBox boundingBox;
};
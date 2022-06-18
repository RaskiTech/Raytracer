#pragma once
#include "DataUtility.h"
#include <vector>

struct Object {
	Object(Material mat) : material(mat) {}
	Object() = default;
	virtual ~Object() = default;

	virtual bool Intersect(const Ray& ray, HitInfo& hitInfo) const = 0;
	
	// Return: Has bounding box
	virtual bool GetBoundingBox(BoundingBox& outBox) const = 0;

	Material material;
};

struct Sphere : public Object {
	Sphere(glm::vec3 pos, float radius, Material mat) : Object(mat), pos(pos), radius(radius) {}

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	glm::vec3 pos{ 0 };
	float radius = 0;
};

struct AxisAlignedCube : public Object {
	AxisAlignedCube(glm::vec3 pos, float radius, Material mat) : Object(mat), minCoord(pos - radius), maxCoord(pos + radius) {}
	AxisAlignedCube(glm::vec3 minPos, glm::vec3 maxPos, Material mat) : Object(mat), minCoord(minPos), maxCoord(maxPos) {}

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	glm::vec3 minCoord;
	glm::vec3 maxCoord;
};

struct YPlane : public Object {
	YPlane(float yPos, Material mat) : Object(mat), yPos(yPos) { if (yPos == 0) this->yPos = 0.001f; }

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	float yPos = 0.1f;
};

struct Fog : public Object {
	Fog(glm::vec3 pos, float radius, float density, std::shared_ptr<Texture> texture)
		: boundary(AxisAlignedCube(pos, radius, Material())), negInverseDensity(-1 / density), Object(Material::CreateIsotropic(texture)) {}

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override;

	AxisAlignedCube boundary;
	float negInverseDensity;
};

struct BVH_Node : public Object {
	BVH_Node() = default;
	BVH_Node(const std::vector<Object*>& scrObjects) : BVH_Node(scrObjects, 0, (int)scrObjects.size()) {}
	BVH_Node(const std::vector<Object*>& scrObjects, int start, int end);
	~BVH_Node();

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { outBox = boundingBox; return true; }

	Object* left = nullptr;
	Object* right = nullptr;
	BoundingBox boundingBox;
};

struct Vertex {
    glm::vec3 position;
    glm::vec2 texcoord;
    glm::vec3 normal;
};

struct Triangle : public Object {
	Triangle(Vertex v1, Vertex v2, Vertex v3) : Triangle(v1, v2, v3, Material()) {}
	Triangle(Vertex v1, Vertex v2, Vertex v3, Material mat)
		: Object(mat), vertices{v1, v2, v3}, 
		box(BoundingBox(glm::min(v1.position, glm::min(v2.position, v3.position)), glm::max(v1.position, glm::max(v2.position, v3.position)))) {}

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { outBox = box; return true; }

	Vertex vertices[3];
	BoundingBox box;
};

struct PolygonMesh : public Object {
    PolygonMesh(std::string pathToObjFile, float size, Material material);

	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { return triangles->GetBoundingBox(outBox); }

	BVH_Node* triangles;
};

struct ApplyYRotation : public Object {
	ApplyYRotation(float degress, Object* target);
	~ApplyYRotation() { delete target; }
	
	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { outBox = box; return true; }

	Object* target;
	float angle;
	float sinTheta;
	float cosTheta;
	bool boxExists;
	BoundingBox box;
	glm::vec3 pivot;
};
struct ApplyZRotation : public Object {
	ApplyZRotation(float degress, Object* target);
	~ApplyZRotation() { delete target; }
	
	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { outBox = box; return true; }

	Object* target;
	float angle;
	float sinTheta;
	float cosTheta;
	bool boxExists;
	BoundingBox box;
	glm::vec3 pivot;
};
struct ApplyXRotation : public Object {
	ApplyXRotation(float degress, Object* target);
	~ApplyXRotation() { delete target; }
	
	bool Intersect(const Ray& ray, HitInfo& hitInfo) const override;
	bool GetBoundingBox(BoundingBox& outBox) const override { outBox = box; return true; }

	Object* target;
	float angle;
	float sinTheta;
	float cosTheta;
	bool boxExists;
	BoundingBox box;
	glm::vec3 pivot;
};

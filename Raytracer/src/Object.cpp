#include "Object.h"
#include <iostream>
#include <algorithm>
#include <gtc/constants.hpp>

bool Sphere::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	glm::vec3 distance = ray.pos - pos;
	float p1 = -glm::dot(ray.direction, distance);
	float p2sqr = p1 * p1 - glm::dot(distance, distance) + radius * radius;
	if (p2sqr < 0)
		return false;
	
	hitInfo.distance = p1 - sqrt(p2sqr);
	if (hitInfo.distance < 0)
		return false;

	hitInfo.point = ray.pos + hitInfo.distance * ray.direction;
	hitInfo.normal = glm::normalize(hitInfo.point - pos);
	hitInfo.object = (Object*)this;

	hitInfo.uv = glm::vec2(glm::atan(hitInfo.normal.x, hitInfo.normal.z) / (2*glm::pi<float>()) + 0.5f, hitInfo.normal.y * -0.5f + 0.5f);

	return true;
}
bool Sphere::GetBoundingBox(BoundingBox& outBox) const {
	outBox = BoundingBox(
		pos - glm::vec3(radius, radius, radius),
		pos + glm::vec3(radius, radius, radius)
	);
	return true;
}
bool AxisAlignedCube::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	std::cerr << "TODO: Alix aligned cube intersect function." << std::endl;
	return false;
}
bool AxisAlignedCube::GetBoundingBox(BoundingBox& outBox) const {
	outBox = BoundingBox(
		pos - glm::vec3(radius, radius, radius),
		pos + glm::vec3(radius, radius, radius)
	);
	return true;
}

bool YPlane::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	float t = -(ray.pos.y - yPos) / ray.direction.y;
	if (t < 1e-3)
		return false;

	hitInfo.distance = t;
	hitInfo.point = ray.pos + t * ray.direction;
	hitInfo.normal = { 0.0f, 1.0f, 0.0f };
	hitInfo.object = (Object*)this;
	return true;
}

bool YPlane::GetBoundingBox(BoundingBox& outBox) const {
	return false;
}


BoundingBox GetSurroundingBox(BoundingBox box0, BoundingBox box1) {
	glm::vec3 small(fmin(box0.minCoord.x, box1.minCoord.x),
		fmin(box0.minCoord.y, box1.minCoord.y),
		fmin(box0.minCoord.z, box1.minCoord.z));

	glm::vec3 big(fmax(box0.maxCoord.x, box1.maxCoord.x),
		fmax(box0.maxCoord.y, box1.maxCoord.y),
		fmax(box0.maxCoord.z, box1.maxCoord.z));

	return BoundingBox(small, big);
}

inline bool CompareObjectAxis(const Object* a, const Object* b, int axis) {
	BoundingBox box_a;
	BoundingBox box_b;

	if (!a->GetBoundingBox(box_a) || !b->GetBoundingBox(box_b))
		std::cerr << "No bounding box in bvh_node constructor.\n";

	return box_a.minCoord[axis] < box_b.minCoord[axis];
}
bool box_x_compare(const Object* a, const Object* b) { return CompareObjectAxis(a, b, 0); }
bool box_y_compare(const Object* a, const Object* b) { return CompareObjectAxis(a, b, 1); }
bool box_z_compare(const Object* a, const Object* b) { return CompareObjectAxis(a, b, 2); }

BVH_Node::BVH_Node(const std::vector<Object*>& scrObjects, int start, int end) {
	/*
	1. randomly choose an axis
	2. sort the primitives (using std::sort)
	3. put half in each subtree
	*/

	auto objects = scrObjects; // Create a modifiable array of the source scene objects

	int axis = (int)(Random01() * 3);
	auto comparator = (axis == 0) ? box_x_compare
					: (axis == 1) ? box_y_compare
								  : box_z_compare;

	int object_span = end - start;

	if (object_span == 1) {
		left = right = objects[start];
	}
	else if (object_span == 2) {
		if (comparator(objects[start], objects[start + 1])) {
			left = objects[start];
			right = objects[start + 1];
		}
		else {
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else {
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		auto mid = start + object_span / 2;
		left = new BVH_Node(objects, start, mid);
		right = new BVH_Node(objects, mid, end);
	}

	BoundingBox box_left, box_right;

	if (!left->GetBoundingBox(box_left) || !right->GetBoundingBox(box_right))
		std::cerr << "No bounding box in bvh_node constructor.\n";

	boundingBox = GetSurroundingBox(box_left, box_right);
}

BVH_Node::~BVH_Node() {
	if (left == right) {
		delete left;
		return;
	}

	if (left != nullptr)
		delete left;
	if (right != nullptr)
		delete right;
}


bool BVH_Node::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	if (!boundingBox.DoesRayHit(ray))
		return false;
	
	bool hit_left = left->Intersect(ray, hitInfo);
	HitInfo info2;
	bool hit_right = right->Intersect(ray, info2);
	if (hit_right && (!hit_left || (hit_left && info2.distance < hitInfo.distance)) )
		hitInfo = info2;

	return hit_left || hit_right;
}
 
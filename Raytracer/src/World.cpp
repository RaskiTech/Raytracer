#include "World.h"
#include "DataUtility.h"
#include "Object.h"
#include "Constants.h"

#include <iostream>
#include <array>
#include <glm.hpp>
#include <gtx/compatibility.hpp>

//////////////////////////////////
///////// Scene objects //////////
//////////////////////////////////

std::vector<Object*> CreateNoBoundingBoxObjects() {
	std::vector<Object*> objs = std::vector<Object*>();
	objs.push_back(new YPlane(0, Material::CreateDiffuse({ 0.9f, 0.9f, 0.9f })));
	return objs;
}
BVH_Node CreateBoundingBoxObjects() {
	std::vector<Object*> objects = std::vector<Object*>();
	objects.push_back(new Sphere{ { 8, 2, -4}, 2, Material::CreateDiffuse({ 1.0f, 1.0f, 1.0f}) });
	for (int i = 0; i < 30; i++)
		objects.push_back(new Sphere{ glm::vec3{i * 2.5f, glm::sin(i * 78.0f) * 1.9f + 1.0f, 15.0f * glm::sin(i * 34.4f)}, glm::sin(i * 78.0f) * 1.9f + 1.0f, Material::CreateDiffuse({glm::sin(i), glm::sin(i * 1.7f), glm::sin(i * 3.3f)}) });

	// This object now owns the pointers, and is responsible for deleting them
	return BVH_Node(objects);
}

World::World()
  : camera({ glm::vec3{ 1, 3, 4 }, 
	glm::vec3{ 3, -0.5f, -3.0f } }), 
	skybox("src/stb_image/skybox12.png"), 
	rootNode(CreateBoundingBoxObjects()),
	noBoundingBoxObjects(CreateNoBoundingBoxObjects())
{
	lightVector = glm::normalize(lightVector);
}

World::~World() {
	for (Object* obj : noBoundingBoxObjects)
		delete obj;
}

glm::u8vec3 World::CalculateColorForScreenPosition(int x, int y) {
	Ray ray;
	glm::vec2 onePixelOffset = { -(1.0f / WINDOW_WIDTH) * ((float)WINDOW_WIDTH / WINDOW_HEIGHT) * FIELD_OF_VIEW, -(1.0f / WINDOW_HEIGHT) * FIELD_OF_VIEW };
	glm::vec2 offset = { -((float)x / WINDOW_WIDTH - 0.5f) * ((float)WINDOW_WIDTH/WINDOW_HEIGHT) * FIELD_OF_VIEW, -((float)y / WINDOW_HEIGHT - 0.5f) * FIELD_OF_VIEW };
	glm::vec3 color{ 0 };

	for (int x = 0; x < SAMPLES_PER_PIXEL_AXIS; x++) {
		for (int y = 0; y < SAMPLES_PER_PIXEL_AXIS; y++) {
			// anti aliasing
			ray.direction = camera.GetForwardVector();
			ray.direction += camera.GetUDirection() * (offset.x + onePixelOffset.x * ((float)x / SAMPLES_PER_PIXEL_AXIS))
						   + camera.GetVDirection() * (offset.y + onePixelOffset.y * ((float)y / SAMPLES_PER_PIXEL_AXIS));
			ray.direction = glm::normalize(ray.direction);

			// depth of field
			glm::vec2 offset = GetRandomUnitCirclePoint() * DEPTH_OF_FIELD_INTENSITY;
			glm::vec3 worldOffset = camera.GetUDirection() * offset.x + camera.GetVDirection() * offset.y;
			ray.pos = camera.pos;
			ray.pos += worldOffset;
			ray.direction -= worldOffset / FOCUS_DISTANCE;
			ray.direction = glm::normalize(ray.direction);

			color += GetRayColor(ray, LIGHT_BOUNCE_AMOUNT);
		}
	}
	color /= SAMPLES_PER_PIXEL_AXIS * SAMPLES_PER_PIXEL_AXIS;

	// Gamma color correct (for diffuse materials)
	color = glm::sqrt(color);

	glm::u8vec3 color8Bit = { (char)(color.r * 256), (char)(color.g * 256), (char)(color.b * 256) };
	return color8Bit;
}

glm::vec3 World::GetRayColor(const Ray& ray, int bounceAmount) {
	glm::vec3 hitPoint;
	glm::vec3 hitNormal;
	Object* hitObj = nullptr;
	float distance = std::numeric_limits<float>::max();
	rootNode.Intersect(ray, hitObj, hitPoint, hitNormal, distance);
	{
		glm::vec3 thisHitPoint;
		glm::vec3 thisHitNormal;
		Object* thisObj;
		float thisDistance;
		//*
		for (int i = 0; i < noBoundingBoxObjects.size(); i++) {
			if (!noBoundingBoxObjects[i]->Intersect(ray, thisObj, thisHitPoint, thisHitNormal, thisDistance))
				continue;

			if (thisDistance < distance) {
				hitObj = thisObj;
				hitPoint = thisHitPoint;
				hitNormal = thisHitNormal;
				distance = thisDistance;
			}
		}
		//*/
	}

	if (hitObj == nullptr)
		return GetSkyboxPixel(ray.direction);

	const Material& material = hitObj->material;
	switch (material.materialType)
	{
		case MaterialType::Diffuse: {
			const float lightDarknerFactor = 0.5f;

			if (bounceAmount == 0)
				return glm::vec3{ 0 };

			Ray reflectedRay;
			reflectedRay.direction = glm::normalize(hitNormal + 2.0f * GetRandomUnitSpherePoint());
			reflectedRay.pos = hitPoint;

			glm::vec3 pixelColor = lightDarknerFactor * GetRayColor(reflectedRay, bounceAmount - 1) * material.color;

			return pixelColor;
		}
		case MaterialType::Metal: {
			float shadowMultiplier = 1.0f - (1.0f - glm::clamp(glm::dot(hitNormal, lightVector) + 1.0f, 0.0f, 1.0f)) * SELF_SHADOW_INTENSITY;
			if (bounceAmount == 0 || material.reflectiveness == 0)
				return material.color * shadowMultiplier;

			Ray reflectedRay;
			reflectedRay.pos = hitPoint;
			reflectedRay.direction = ray.direction - 2.0f * hitNormal * glm::dot(ray.direction, hitNormal);

			glm::vec3 pixelColor = (1.0f - material.reflectiveness) * material.color
				+ material.reflectiveness * GetRayColor(reflectedRay, bounceAmount - 1);
			return pixelColor * shadowMultiplier;
		}
		default:
			return glm::vec3{ 0 };
	}
}

glm::vec3 World::GetSkyboxPixel(const glm::vec3& direction) {
	glm::vec2 pixel = glm::vec2{ (0.5f + glm::atan(direction.x, direction.z) / (glm::two_pi<float>())) * skybox.size.x, (0.5f - glm::asin(direction.y) / glm::pi<float>()) * skybox.size.y };
	int intPixelX = (int)glm::floor(pixel.x);
	int intPixelY = (int)glm::floor(pixel.y);

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

	glm::vec3 col = GetSkyboxPixel((int)pixel.x, (int)pixel.y) * originalMultiplier;
	col += GetSkyboxPixel(otherX, (int)pixel.y) * otherXMultiplier;
	col += GetSkyboxPixel((int)pixel.x, otherY) * otherYMultiplier;
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
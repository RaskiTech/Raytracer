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
	objs.push_back(new YPlane(0, Material::CreateMetal(0.3f, Texture::CreateCheckered({1.0f, 1.0f, 1.0f}, {0.2f, 0.6f, 0.3f}))));
	return objs;
}
BVH_Node CreateBoundingBoxObjects() {
	std::vector<Object*> objects = std::vector<Object*>();
	objects.push_back(new Sphere{ {8, 2, -4}, 2, Material::CreateDiffuse(Texture::CreateCheckered({ 0.6f, 0.3f, 0.2f }, { 1.0f, 1.0f, 1.0f})) });
	objects.push_back(new Sphere{ {2, 2, -4}, 2, Material::CreateDiffuse(Texture::CreateFromImage("src/stb_image/Earth.png")) });
	for (int i = 0; i < 20; i++)
		objects.push_back(new Sphere{ glm::vec3{i * 2.5f+10, glm::sin(i * 78.0f) * 1.9f + 1.0f, 15.0f * glm::sin(i * 34.4f)}, glm::sin(i * 78.0f) * 1.9f + 1.0f, Material::CreateDiffuse(Texture::CreateColored({ glm::sin(i), glm::sin(i * 1.7f), glm::sin(i * 3.3f) }))});

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

	glm::u8vec3 color8Bit = { (char)(color.r * 255), (char)(color.g * 255), (char)(color.b * 255) };
	return color8Bit;
}

glm::vec3 World::GetRayColor(const Ray& ray, int bounceAmount) {
	HitInfo hitInfo;
	rootNode.Intersect(ray, hitInfo);
	{
		HitInfo thisHitInfo;
		//*
		for (int i = 0; i < noBoundingBoxObjects.size(); i++) {
			if (!noBoundingBoxObjects[i]->Intersect(ray, thisHitInfo))
				continue;

			if (thisHitInfo.distance < hitInfo.distance)
				hitInfo = thisHitInfo;
		}
	}

	if (hitInfo.object == nullptr) {
		glm::vec3 col = GetSkyboxPixel(ray.direction);
		return col;
	}

	const Material& material = hitInfo.object->material;
	switch (material.materialType)
	{
		case MaterialType::Diffuse: {
			const float lightDarknerFactor = 0.5f;

			if (bounceAmount == 0)
				return glm::vec3{ 0 };

			Ray reflectedRay;
			reflectedRay.direction = glm::normalize(hitInfo.normal + 2.0f * GetRandomUnitSpherePoint());
			reflectedRay.pos = hitInfo.point;

			glm::vec3 pixelColor = lightDarknerFactor * GetRayColor(reflectedRay, bounceAmount - 1) * material.texture->GetColorValue(hitInfo.uv, hitInfo.point);

			return pixelColor;
		}
		case MaterialType::Metal: {
			if (bounceAmount == 0 || material.reflectiveness == 0)
				return material.texture->GetColorValue(hitInfo.uv, hitInfo.point);

			Ray reflectedRay;
			reflectedRay.pos = hitInfo.point;
			reflectedRay.direction = ray.direction - 2.0f * hitInfo.normal * glm::dot(ray.direction, hitInfo.normal);

			glm::vec3 pixelColor = (1.0f - material.reflectiveness) * material.texture->GetColorValue(hitInfo.uv, hitInfo.point)
				+ material.reflectiveness * GetRayColor(reflectedRay, bounceAmount - 1);
			return pixelColor;
		}
		default:
			return glm::vec3{ 0 };
	}
}

glm::vec3 World::GetSkyboxPixel(const glm::vec3& direction) {
	glm::vec2 uv = glm::vec2(glm::atan(direction.x, direction.z) / (2*glm::pi<float>()) + 0.5f, direction.y * -0.5f + 0.5f);
	glm::vec2 pixel = { uv.x * skybox.size.x, uv.y * skybox.size.y };

	int intPixelX = (int)glm::floor(pixel.x);
	int intPixelY = (int)glm::floor(pixel.y);

	return GetSkyboxPixel(intPixelX, intPixelY);

	/*
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

	glm::vec3 col = GetSkyboxPixel(intPixelX, intPixelY) * originalMultiplier;
	col += GetSkyboxPixel(otherX, intPixelY) * otherXMultiplier;
	col += GetSkyboxPixel(intPixelX, otherY) * otherYMultiplier;
	col += GetSkyboxPixel(otherX, otherY) * otherXYMultiplier;

	return col;
	//*/
}
glm::vec3 World::GetSkyboxPixel(int x, int y) {
	int index = 3 * (int)(y * skybox.size.x + x);
	uint8_t c1 = skybox.skyboxImageData[index];
	uint8_t c2 = skybox.skyboxImageData[index + 1];
	uint8_t c3 = skybox.skyboxImageData[index + 2];

	float f1 = c1;
	float f2 = c2;
	float f3 = c3;

	return glm::vec3{ f1, f2, f3 } / 255.0f;
}
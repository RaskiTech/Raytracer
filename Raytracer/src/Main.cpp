#include <iostream>
#include "App.h"
#include "World.h"

#include "DataUtility.h"
#include <glm.hpp>

// Don't try to steal my main, SDL
#undef main

/*
void LogBoxText(BoundingBox& box, glm::vec3 rayOrigin, glm::vec3 rayDir, bool shouldHit) {
    Ray ray;
    ray.pos = rayOrigin;
    ray.direction = rayDir;
    std::cout << "Ray (" << ray.pos.x << ", " << ray.pos.y << ", " << ray.pos.z << ") towards ";
    std::cout << "(" << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << ")\n";

    ray.direction = glm::normalize(ray.direction);
    bool hit = box.DoesRayHit(ray);
    std::cout << "Hit: " << hit << (shouldHit == hit ? "" : " THIS IS NOT THE THING IT IS SUPPOSED TO BE! ") << "\n\n";
}
{
    BoundingBox box;
    box.minCoord = glm::vec3{ -1, -1, -1 };
    box.maxCoord = glm::vec3{ 1, 1, 1 };

    std::cout << "Should hit: \n";
    LogBoxText(box, { -2, -2, -2 }, { 1, 1, 1 }, true);
    LogBoxText(box, { 2, -2, -2 }, { -1, 1, 1 }, true);
    LogBoxText(box, { -2, 2, 2 }, { 1, -1, -1 }, true);
    LogBoxText(box, { 2, 2, 2 }, { -1, -1, -1 }, true);

    std::cout << "Shouldn't hit: \n";
    LogBoxText(box, { 2, -2, -2 }, { 1, 1, 1 }, false);
    LogBoxText(box, { 2, -2, -2 }, { -1, -1, -1 }, false);
    LogBoxText(box, { -2, 2, 2 }, { 1, -1, 1 }, false);
    LogBoxText(box, { 2, 2, 2 }, { 1, 1, 1 }, false);

    std::cout << "Edge cases: \n";
    LogBoxText(box, { 2, 2, 2 }, { 1, 0, 1 }, false);
    LogBoxText(box, { 2, 1, 2 }, { 1, 0, 1 }, false);
    LogBoxText(box, { -2, 2, -2 }, { -1, 0, -1 }, false);
    LogBoxText(box, { -2, 1, -2 }, { -1, 0, -1 }, false);
    LogBoxText(box, { 0, 0, 0 }, { 1, 1, 1 }, true);
    LogBoxText(box, { 0, 0, 0 }, { -1, -1, -1 }, true);
}
*/

int main()
{

    RayTracer* app = new RayTracer();
    app->Loop();
    delete app;

    return 0;
}
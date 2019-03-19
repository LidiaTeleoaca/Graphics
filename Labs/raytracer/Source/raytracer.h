#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <glm/gtx/quaternion.hpp>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
#define FULLSCREEN_MODE false

struct Intersection
{
  glm::vec4 position;
  glm::vec4 direction;
  float distance;
  int triangleIndex;
  glm::vec3 color;
};

void Update();
void Draw(screen* screen, const std::vector<Triangle>& triangles);
glm::vec4 reflected_ray(glm::vec4 incident, glm::vec4 normal);
glm::vec4 refracted_ray(const Intersection& intersection, const std::vector<Triangle>& triangles, glm::vec4 incident);
float compute_fresnel(glm::vec4 incident, glm::vec4 normal, float ior);
glm::vec3 computeLights(Intersection intersection, const std::vector<Triangle>& triangles);
float specular(const Intersection& intersection, const std::vector<Triangle>& triangles);
glm::vec3 DirectLight(const Intersection& i, const std::vector<Triangle>& triangles);
bool ClosestIntersection(
  glm::vec4 s,
  glm::vec4 d,
  const std::vector<Triangle>& triangles,
  bool ignore,
  Intersection& closestIntersection );
glm::vec3 reflected_light(const Intersection& i, const std::vector<Triangle>& triangles);
glm::vec3 transparent(const Intersection& intersection, const std::vector<Triangle>& triangles, int depth);
glm::vec3 compute_Colour(glm::vec4 start, glm::vec4 direction, const std::vector<Triangle>& triangles, bool ignore, int depth);

#endif

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <glm/gtx/quaternion.hpp>
#include "raytracer.h"

using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */
float EPSILON = 0.001f;                     //for objects to not self-intersect
float air_ior = 1.0f;                       // aprox. of index of refraction of air
float focalLength = SCREEN_HEIGHT;          // focal lenght of the camera
vec4 cameraPos(0,0,-3,1.0);                 // position of the camera
vec4 lightPos( 0, -0.5, -0.2f, 1.0 );        // position of light source
vec3 lightColor = 14.0f * vec3( 1, 0.7, 0.8 );  // intensity and color of the light source
vec3 indirectLight = 0.4f*vec3( 1, 1, 1 );  // intensity and color of indirect light
int MAXDEPTH = 25;                          // maximum depth of refraction rays
int specular_power = 20;                    //the power that the specular highlight is raised to
float diffuse_part = 0.8;                   //the diffuse part of a specular object
float specular_part = 0.2;                  //the specular part of a specular object

vec3 zero3 = vec3(0.0f, 0.0f, 0.0f);        //zero vec3 for convenience
vec4 zero4 = vec4(0.0f,0.0f,0.0f,0.0f);     //zero vec4 for convenience
glm::quat rotQuat = glm::quat(vec3(0,0,0)); //quaternion used for computing rotation matrix
mat4 R = glm::toMat4(rotQuat);              //rotation matrix

//main function
int main( int argc, char* argv[] )
{

  std::vector<Triangle> triangles;

  LoadTestModel(triangles);

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  while( NoQuitMessageSDL() )
    {
      Update();
      Draw(screen, triangles);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, const std::vector<Triangle>& triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  // used so that refraction doesn't segfault
  bool ignore = false;
  for(int i=0; i<SCREEN_HEIGHT; i++)
    {
      for(int j = 0; j<SCREEN_WIDTH; j++)
      {
        vec4 d = vec4(i - SCREEN_HEIGHT/2, j - SCREEN_WIDTH/2 , focalLength, 1.0);
        d = rotQuat * d;
        vec4 s = cameraPos;
        vec3 color = compute_Colour(s,glm::normalize(d),triangles, ignore, 0);
        PutPixelSDL(screen, i, j, color);
      }
    }
}


/*Place updates of parameters here*/
void Update()
{

  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  float yaw = 0;
  float xaw = 0;

  /*Good idea to remove this*/
  //std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update camera*/
  vec4 fwd  (R[2][0], R[2][1], R[2][2], 0);
  vec4 down (R[1][0], R[1][1], R[1][2], 0);
  vec4 right(R[0][0], R[0][1], R[0][2], 0);

  //update camera based on Key pressed
  const uint8_t* keystate = SDL_GetKeyboardState( 0 );

  // Position
  if( keystate[SDL_SCANCODE_UP] )
  {
    cameraPos += 0.0005f * dt * fwd;
  }
  if( keystate[SDL_SCANCODE_DOWN] )
  {
    cameraPos -= 0.001f * dt * fwd;
  }
  if( keystate[SDL_SCANCODE_LEFT] )
  {
    cameraPos -= 0.001f * dt * right;
  }
  if( keystate[SDL_SCANCODE_RIGHT] )
  {
    cameraPos += 0.001f * dt * right;
  }

  if( keystate[SDL_SCANCODE_N] )
  {
    cameraPos += 0.001f * dt * down;
  }
  if( keystate[SDL_SCANCODE_M] )
  {
    cameraPos -= 0.001f * dt * down;
  }

  // Rotation
  if( keystate[SDL_SCANCODE_F] )
  {
    yaw = -0.001 * dt;
  }
  if( keystate[SDL_SCANCODE_H] )
  {
    yaw = 0.001 * dt;
  }
  if( keystate[SDL_SCANCODE_T] )
  {
    xaw = 0.001 * dt;
  }
  if( keystate[SDL_SCANCODE_G] )
  {
    xaw = -0.001 * dt;
  }

  // Update light position
  if( keystate[SDL_SCANCODE_W] )
  {
    lightPos +=  0.001f * dt * fwd;
  }
  if( keystate[SDL_SCANCODE_S] )
  {
    lightPos -=  0.001f * dt * fwd;
  }
  if( keystate[SDL_SCANCODE_D] )
  {
    lightPos +=  0.001f * dt * right;
  }
  if( keystate[SDL_SCANCODE_A] )
  {
    lightPos -=  0.001f * dt * right;
  }
  if( keystate[SDL_SCANCODE_Q] )
  {
    lightPos +=  0.001f * dt * down;
  }
  if( keystate[SDL_SCANCODE_E] )
  {
    lightPos -=  0.001f * dt * down;
  }

  //Update Rotation Matrix
  rotQuat *= glm::quat(vec3(xaw,yaw,0));
  R = glm::toMat4(rotQuat);

  xaw = 0;
  yaw = 0;
}

/*
  Computes the closes intersection of a ray, given a start position and direction
  Returns true if it intersects with a triangle
  Updates the closestIntersection struct with the information of the closest intersection found
*/
bool ClosestIntersection(vec4 s, vec4 d, const std::vector<Triangle>& triangles, bool ignore, Intersection& closestIntersection ){
  float m = std::numeric_limits<float>::max();
  bool result = false;

  for(unsigned int i = 0; i < triangles.size(); i++){
    Triangle triangle = triangles[i];

    if (glm::dot(d, triangle.normal) > 0.0f){
      continue;
    }
    closestIntersection.direction = glm::normalize(d);

    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;

    vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
    vec3 b  = vec3(s.x-v0.x,s.y-v0.y,s.z-v0.z);
    mat3 A( vec3(-d), e1, e2 );
    vec3 x = glm::inverse( A ) * b;

    float t = x.x;
    float u = x.y;
    float v = x.z;



    if((t > -EPSILON) & (u > -EPSILON) & (v > -EPSILON) & ((u+v) < 1.0f + EPSILON))
    {
      if (t < m)
      {
        m = t;
        vec4 r = s + t*d;
        closestIntersection.position = r;
        closestIntersection.distance = t;
        closestIntersection.triangleIndex = i;

        closestIntersection.color = triangle.color;
        result = true;
      }
    }
  }

  return result;

}

vec3 compute_Colour(vec4 start, vec4 direction, const std::vector<Triangle>& triangles, bool ignore, int depth ){


  // If bigger than a max depth, stop
  if(depth > MAXDEPTH){
    return zero3;
  }

  vec3 color = zero3;
  Intersection intersection;
  // Cast ray
  if(!ClosestIntersection(start, direction, triangles, ignore,intersection))
    return zero3;

  ignore = true;

  vec4 pos = intersection.position + direction * EPSILON;
  vec3 lights = computeLights(intersection, triangles);
  const Triangle& triangle = triangles[intersection.triangleIndex];

  // Compute color of the object hit by the ray, depending on its properties //

  // Diffuse objects
  if(!triangle.ior && !triangle.reflective){
    return triangle.color * lights;
  }

  vec3 reflection = zero3;
  vec4 reflection_ray = reflected_ray(intersection.direction, triangle.normal);

  // Reflective objects/mirrors
  if(triangle.reflective && !triangle.ior){

    float refl_amount = compute_fresnel(intersection.direction, triangle.normal, 0.0f);
    vec4 reflect_ray_pos = intersection.position + glm::normalize(reflection_ray) * EPSILON;
    /*vec4 reflect_ray_pos = intersection.position - triangle.normal*EPSILON;
    if(glm::dot(intersection.direction, triangle.normal) < 0.0f){
      reflect_ray_pos = intersection.position + triangle.normal*EPSILON;
    }*/

    reflection += compute_Colour(reflect_ray_pos, reflection_ray, triangles, false,++depth) * refl_amount;
    vec3 reflColour = triangle.color*(1-triangle.reflective) + reflection*triangle.reflective;
    color += reflColour * lights;

  }

  // Reflective and refractive objects (glass, diamond)
  if(triangle.reflective && triangle.ior ){
    vec3 refraction_colour = zero3;
    float refl_amount = compute_fresnel(direction, triangle.normal, triangle.ior);

    if( refl_amount < 1.0f){
      vec4 refraction_ray = refracted_ray(intersection, triangles, direction);
      refraction_colour += compute_Colour(pos, glm::normalize(refraction_ray), triangles, true, ++depth);
    }
    if(depth == MAXDEPTH){
      depth = 0;
    }
    vec3 reflection_colour = zero3;
    vec3 spec = zero3;
    vec4 reflect_ray_pos = intersection.position + glm::normalize(reflection_ray)*EPSILON;
    //vec4 reflect_ray_pos = intersection.position - triangle.normal*EPSILON;
    /*if(glm::dot(intersection.direction, triangle.normal) < 0.0f){
      reflect_ray_pos = intersection.position + triangle.normal*EPSILON;
    }*/
    reflection = compute_Colour(reflect_ray_pos, reflection_ray, triangles, ignore,++depth);
    reflection_colour = triangle.color*(1.0f - triangle.reflective) + reflection*triangle.reflective;
    reflection_colour = reflection_colour*lights;
    color = reflection_colour * refl_amount + refraction_colour * (1.0f - refl_amount);
    color = triangle.color * (1.0f - triangle.transparent) + color * triangle.transparent;

    if(DirectLight(intersection, triangles) != zero3){
      spec = lightColor * specular(intersection, triangles) ;
      vec3 light_spec = color * (lights*diffuse_part + spec * specular_part);
      color = color * diffuse_part + color * light_spec *specular_part;

    }
  }

  return color;
}

//computes the amount of reflectiveness
float compute_fresnel(vec4 incident, vec4 normal, float ior){
  float cosi = glm::dot(normal, glm::normalize(incident));
  float ior1 = air_ior;
  float ior2 = ior;
  if(cosi > 0.0f){
    std::swap(ior1, ior2);
  }
  else{
    cosi = -cosi;
  }
  //float sint = ior1/ior2 * sqrtf(max(0.0f, 1.0f - cosi * cosi));
  float sint = ior1/ior2 * (1.0f - cosi*cosi/4.0f); // ---> used instead of sqrt for efficiency
  if(sint >= 1.0f ){
    return 1.0f;
  }

  else{
      //float cost = sqrtf(max(0.0f, 1.0f - sint * sint));
      float cost = 1.0f - sint*sint/4.0f; // ---> used instead of sqrt for efficiency
      if(cost < 0.0f){
        cost = -cost;
      }

      float rs = ((ior2 * cosi) - (ior1 * cost))/((ior2 * cosi) + (ior1 * cost));
      float rp = ((ior1 * cosi) - (ior2 * cost))/((ior1 * cosi) + (ior2 * cost));

      return (rs*rs + rp*rp)/2.0f;
  }
}


// Computes the sum of the lights contributions in the scene
vec3 computeLights(Intersection intersection, const std::vector<Triangle>& triangles){
  vec3 dir = DirectLight(intersection, triangles);
  return dir + indirectLight;
}

vec4 refracted_ray(const Intersection& intersection, const std::vector<Triangle>& triangles, vec4 incident){
  vec4 normal = triangles[intersection.triangleIndex].normal;
  float dot = glm::dot(normal, incident);
  const Triangle& triangle = triangles[intersection.triangleIndex];
  float ior1 = air_ior;
  float ior2 = triangle.ior;

  if(dot < 0){
    dot = -dot;
  }
  else{
    normal = -normal;
    std::swap(ior1, ior2);
  }
  float ior = ior1/ior2;
  float term = 1 - (ior*ior)*(1 - dot*dot);
  if(term <= 0){
    return zero4;
  }
  else{
    return ior*incident + normal*(ior*dot - sqrt(term));
  }
}

// Computes the ray that gets reflected
vec4 reflected_ray(vec4 incident,vec4 normal){
  return incident - 2.0f * glm::dot(incident,normal) *  normal ;
}


//computes the specular amount at a point
float specular(const Intersection& intersection, const std::vector<Triangle>& triangles){
  const Triangle& triangle = triangles[intersection.triangleIndex];
  vec4 light_incident = lightPos - intersection.position;
  vec4 incident = intersection.position - cameraPos;
  vec4 reflection_ray = - reflected_ray(light_incident, triangle.normal);

  float spec = std::max(0.0f,glm::dot(glm::normalize(incident), reflection_ray));
  if(spec){
    return pow(spec,specular_power);
  }

  return 0.0f;
}

// Computes direct lighting. Resturn a color that should be applied on the base color of a surfaces
vec3 DirectLight(const Intersection& intersection, const std::vector<Triangle>& triangles) {
  vec4 incident = lightPos - intersection.position;

  float distanceToLight = glm::length(incident);
  const Triangle& triangle = triangles[intersection.triangleIndex];
  Intersection lightIntersection;

  bool ignore = true;
  bool intersect = ClosestIntersection(intersection.position, glm::normalize(incident), triangles, ignore, lightIntersection);

  // distanceToLight - 1 used for better results/ for less ojects to be in the shadow
  //if (glm::dot(incident, triangle.normal) < 0.0f){
    if (intersect && lightIntersection.distance < distanceToLight -1.0f) {
      if(!triangle.transparent && triangles[lightIntersection.triangleIndex].transparent > 0.0f){
        return vec3(0.1f,0.1f,0.1f);
      }
        return vec3(0.0f, 0.0f, 0.0f);
  }

  float illumination = std::max(glm::dot(triangle.normal, glm::normalize(incident)), 0.0f) / (6.0 * 3.141 * distanceToLight * distanceToLight) ;

  return illumination * lightColor;
}

#ifndef RASTERISER_H
#define RASTERISER_H

#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include "InterpolateAuxiliary.h"
#include <stdint.h>
#include <glm/gtx/quaternion.hpp>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
#define FULLSCREEN_MODE false

class Vertex
{
public:
  glm::vec4 position;
  //vec4 normal;
  //vec2 reflectance;
};

void Update();
void Draw(screen* screen);
void Draw_Scene(screen* screen);
void PixelShader( screen* screen,const Pixel& p);
void Compute_Blurr(Pixel p, glm::vec3 &color);
void VertexShader( const Vertex& v, Pixel& p );
void ComputeEdge(Pixel a, Pixel b, std::vector<Pixel>& edges);
void DrawLineSDL( screen* screen, Pixel a, Pixel b );
void DrawPolygonEdges( screen* screen, const std::vector<Vertex>& vertices);
void DrawPolygon( screen* screen, const std::vector<Vertex>& vertices );
void DrawRows(
  screen* screen,
  const std::vector<Pixel>& leftPixels,
  const std::vector<Pixel>& rightPixels );
void ComputePolygonRows(
  const std::vector<Pixel>& vertexPixels,
  std::vector<Pixel>& leftPixels,
  std::vector<Pixel>& rightPixels );
void TransformationMatrix(glm::mat4x4& transfmatrix, glm::mat4x4 cam, glm::mat4 rotation);

#endif

#ifndef INTERPOLATE_AUXILIARY_H
#define INTERPOLATE_AUXILIARY_H
#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "SDLauxiliary.h"
#include <stdint.h>
#include <vector>

struct Pixel
{
public:
  int x;
  int y;
  float zinv;
  glm::vec4 pos3d;
  glm::vec3 color;
  //glm::vec3 illumination;
};

void Interpolate( Pixel a, Pixel b, std::vector<Pixel>& result );

template <class T>
void Interpolate( T a, T b, std::vector<T>& result )
{
  T step( a );

  for(unsigned int i=0; i<result.size(); i++ )
  {
    result[i] = step;
    step += (float) (1.0/(result.size() - 1)) * (b -a);
  }
}

#endif

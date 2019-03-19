#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "SDLauxiliary.h"
#include <stdint.h>
#include <vector>
#include "InterpolateAuxiliary.h"

void Interpolate( Pixel a, Pixel b, std::vector<Pixel>& result )
{
  glm::vec3 step;
  step.x = a.x;
  step.y = a.y;
  step.z = a.zinv;

  std::vector<glm::vec4> pos3d = std::vector<glm::vec4>(result.size());
  std::vector<glm::vec3> color = std::vector<glm::vec3>(result.size());
  Interpolate(a.pos3d, b.pos3d, pos3d);
  Interpolate(a.color, b.color, color);

  for(unsigned int i=0; i<result.size(); i++ )
  {
    result[i].x = step.x;
    result[i].y = step.y;
    result[i].zinv = step.z;
    result[i].pos3d = pos3d[i];
    result[i].color = color[i];

    step.x+= (b.x-a.x)*(1.0/(result.size() - 1)) ;
    step.y+= (b.y-a.y)*(1.0/(result.size() - 1)) ;
    step.z+= (b.zinv-a.zinv)*(1.0/(result.size() - 1)) ;
  }
}

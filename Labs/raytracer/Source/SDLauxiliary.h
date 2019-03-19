#ifndef SDL_AUXILIARY_H
#define SDL_AUXILIARY_H

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <stdint.h>

typedef struct{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  int height;
  int width;
  uint32_t *buffer;
} screen;

screen* InitializeSDL( int width, int height, bool fullscreen = false );
bool NoQuitMessageSDL();
void PutPixelSDL( screen *s, int x, int y, glm::vec3 color );
void SDL_Renderframe(screen *s);
void KillSDL(screen* s);
void SDL_SaveImage(screen *s, const char* filename);

#endif

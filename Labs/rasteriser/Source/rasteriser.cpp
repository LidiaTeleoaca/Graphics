#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include "InterpolateAuxiliary.h"
#include <stdint.h>
#include <glm/gtx/quaternion.hpp>
#include "rasteriser.h"

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

//Triangles
std::vector<Triangle> triangles;
vec4 currentNormal;
vec3 currentColor;
//vec3 currentReflectance;

//Light
vec4 lightPos(0.0,-0.5,-0.7,1.0);
float light_amount = 7.0f;
vec3 lightPower = light_amount*vec3( 1, 1, 1 );
vec3 indirectLight = 0.6f*vec3( 1, 1, 1 );
vec3 fogcolor = vec3(0.48,0.52,0.60);
bool draw = false;

glm::mat3 kernel = glm::transpose( glm::mat3(
                      1/9, 1/9, 1/9,
                      1/9, 1/9, 1/9,
                      1/9, 1/9, 1/9
                    ));

//Rotation Matrix
glm::quat rotQuat = glm::quat(vec3(0,0,0));
glm::mat4 R = glm::toMat4(rotQuat);

//Transformation matrix
glm::mat4x4 transformationMatrix = glm::mat4x4(1.0f);

//Depth Buffer
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
std::vector<vec3> colors = std::vector<vec3>(SCREEN_WIDTH*SCREEN_HEIGHT);
int THRESHOLD = 4.0f;
float max_z = 4.0f;
//Camera
float focalLength = SCREEN_HEIGHT;
float no_fog = 1.0f;
float only_fog = 4.0f;
vec4 cameraPos = vec4(0,0,-3.001,1);
glm::mat4x4 cameraMatrixPos = glm::transpose(glm::mat4x4(
                                  1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 3.001,
                                  0, 0, 0, 1
                              ));



//main function
int main( int argc, char* argv[] )
{

  LoadTestModel(triangles);

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  while( NoQuitMessageSDL() )
    {
      Update();
      Draw(screen);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen)
{
  max_z = 0.0f;
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  for( int y=0; y<SCREEN_HEIGHT; ++y )
    for( int x=0; x<SCREEN_WIDTH; ++x )
      depthBuffer[y][x] = 0;

  Draw_Scene(screen);
  draw = true;
  Draw_Scene(screen);
}

void Draw_Scene(screen* screen){
  for( uint32_t i=0; i<triangles.size(); ++i )
  {
    std::vector<Vertex> vertices(3);
    const Triangle triangle = triangles[i];
    vertices[0].position = triangle.v0;
    vertices[1].position = triangle.v1;
    vertices[2].position = triangle.v2;
    currentNormal = triangles[i].normal;
    currentColor = triangles[i].color;

    DrawPolygon(screen, vertices);
  }
}

// Draws rows between right and left pixels of a triangle
void DrawRows( screen* screen, const std::vector<Pixel>& leftPixels,const std::vector<Pixel>& rightPixels){
  for (unsigned int i = 0; i < leftPixels.size(); i++){
    DrawLineSDL(screen, leftPixels[i], rightPixels[i]);
  }
}


// Draws a line between two pixels
void DrawLineSDL( screen* screen, Pixel a, Pixel b ){
  std::vector<Pixel> line;
  ComputeEdge(a, b, line);

    for (int i = 0; i < (int)line.size(); i++ ){
      Pixel p = line[i];
      vec4 incident = lightPos - p.pos3d;
      float distanceToLight = glm::length(incident);
      float illumination = std::max(glm::dot(currentNormal, glm::normalize(incident)) , 0.0f) / (4.0 * 3.141 * distanceToLight * distanceToLight) ;
      vec3 directLight = lightPower * illumination;
      if(line[i].x >= 0 && line[i].y >= 0 && line[i].x < SCREEN_WIDTH && line[i].y < SCREEN_HEIGHT){
        float z = 1.0f/p.zinv;
        if(z > max_z){
          max_z = z;
        }
        if( p.zinv >= depthBuffer[p.y][p.x] ){
          depthBuffer[p.y][p.x] = p.zinv;
          colors[p.y*SCREEN_WIDTH + p.x] = p.color * (directLight+indirectLight);
        }
      }
    }

  if(draw){
    for (int i = 0; i < (int)line.size(); i++ ){
      if(line[i].x >= 0 && line[i].y >= 0 && line[i].x < SCREEN_WIDTH && line[i].y < SCREEN_HEIGHT){
        PixelShader(screen, line[i]);
      }
    }
  }
}

// Computes an edge between 2 points of a polygon
void ComputeEdge(Pixel a, Pixel b, std::vector<Pixel>& edge){

  ivec2 delta;
  delta.x = glm::abs( a.x - b.x );
  delta.y = glm::abs( a.y - b.y );
  int pixels = glm::max( delta.x, delta.y ) + 1;
  edge = std::vector<Pixel>( pixels );
  Interpolate(a, b, edge);
}

void PixelShader( screen * screen, const Pixel& p ){
  int x = p.x;
  int y = p.y;

  vec4 incident = lightPos - p.pos3d;
  float distanceToLight = glm::length(incident);

  float illumination = std::max(glm::dot(currentNormal, glm::normalize(incident)) , 0.0f) / (4.0 * 3.141 * distanceToLight * distanceToLight) ;
  vec3 directLight = lightPower * illumination;
  vec3 color = p.color * (directLight + indirectLight);
  if( p.zinv >= depthBuffer[y][x] ){

    depthBuffer[y][x] = p.zinv;

    Compute_Blurr(p, color);

    PutPixelSDL( screen, x, y, color);
  }
}

//blurr based on the depth z
void Compute_Blurr(Pixel p, vec3 &color){
  float z = 1.0f/p.zinv;
  for(int i = 0; i < 3; i++ ){
    for(int j = 0; j < 3; j++ ){
      if(p.zinv < 0.3){
        float amount = std::min(1.0f, p.zinv );
        if(i == 1 && j == 1)
          kernel[i][j] = amount;
        else
          kernel[i][j] = (1.0f - amount)/8.0f;
      }
      else if (p.zinv > 0.7){
        float amount = std::max(1.0f/9.0f, z/(max_z*max_z));

        if(i == 1 && j == 1)
          kernel[i][j] = amount;
        else
          kernel[i][j] = (1.0f - amount)/8.0f;
      }
      else{
        if(i == 1 && j == 1)
          kernel[i][j] = 1.0f;
        else
          kernel[i][j] = 0.0f;
      }
    }
  }

  vec3 color_sum = vec3(0.0f,0.0f,0.0f);
  if(p.x > 0 && p.y > 0 && p.x < SCREEN_WIDTH - 1 && p.y < SCREEN_HEIGHT - 1){
    for(int i = 0; i < 3; i++ ){
      for(int j = 0; j < 3; j++ ){
        color_sum += colors[(p.y + i - 1) * SCREEN_WIDTH + (p.x + j - 1) ] * kernel[i][j];
      }
    }
    color = color_sum;
  }
}

//Draws a polygon
void DrawPolygon( screen* screen, const std::vector<Vertex>& vertices)
{
  int V = vertices.size();
  std::vector<Pixel> vertexPixels( V );
  for( int i=0; i<V; ++i )
    VertexShader( vertices[i], vertexPixels[i] );
  std::vector<Pixel> leftPixels;
  std::vector<Pixel> rightPixels;
  ComputePolygonRows( vertexPixels, leftPixels, rightPixels );

  DrawRows( screen, leftPixels, rightPixels );
}

/*//draws the edges of a polygon in white
void DrawPolygonEdges( screen* screen, const vector<Vertex>& vertices ) {
  int V = vertices.size();
  // Transform each vertex from 3D world position to 2D image position:
  vector<Pixel> projectedVertices( V );
  for( int i=0; i<V; ++i )
  {
    VertexShader( vertices[i], projectedVertices[i] );
  }
  // Loop over all vertices and draw the edge from it to the next vertex:
  for( int i=0; i<V; ++i )
  {
    int j = (i+1)%V; // The next vertex
    DrawLineSDL( screen, projectedVertices[i], projectedVertices[j] );
  }
}*/

/*Place updates of parameters here*/
void Update()
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;

  /*Good idea to remove this*/
  //std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update camera*/

  float yaw = 0;
  vec4 fwd  (R[2][0], R[2][1], R[2][2], 0);
  vec4 down (R[1][0], R[1][1], R[1][2], 0);
  vec4 right(R[0][0], R[0][1], R[0][2], 0);


  //updates camera position based on key pressed

  const uint8_t* keystate = SDL_GetKeyboardState( 0 );
  if( keystate[SDL_SCANCODE_UP] )
  {
    cameraMatrixPos[3].z -= 0.05;
  }
  if( keystate[SDL_SCANCODE_DOWN] )
  {
    cameraMatrixPos[3].z += 0.05;
  }
  if( keystate[SDL_SCANCODE_LEFT] )
  {
    yaw = -0.001 * dt;
  }
  if( keystate[SDL_SCANCODE_RIGHT] )
  {
    yaw = 0.001 * dt;
  }

  //update light position
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

  //updates rotation
  rotQuat *= glm::quat(vec3(0,yaw,0));
  R = glm::toMat4(rotQuat);
  yaw = 0;
}

// Draws pixels on a 2D screen, based on their 3D location and the camera position
void VertexShader( const Vertex& v, Pixel& projPos ){
  /* Per Vertex Illumination
    vec4 incident = lightPos - v.position;
    float distanceToLight = glm::length(incident);
    float directLight = max(glm::dot(v.normal, glm::normalize(incident)) , 0.0f) / (4.0 * 3.141 * distanceToLight * distanceToLight) ;
    projPos.illumination = (lightPower * directLight) + indirectLight;
  */

  TransformationMatrix(transformationMatrix, cameraMatrixPos, R);
  vec4 new_v = transformationMatrix * v.position  ;
  projPos.zinv = 1.0/new_v.z;
  projPos.x =  int(focalLength * new_v.x * projPos.zinv) + SCREEN_WIDTH/2;
  projPos.y =  int(focalLength * new_v.y * projPos.zinv) + SCREEN_HEIGHT/2;
  projPos.pos3d = v.position * projPos.zinv;


  float fog_amount = std::max((new_v.z - no_fog)/(only_fog - no_fog), 0.0f);
  fog_amount = std::min(fog_amount*fog_amount, 0.95f);

  projPos.color = currentColor* (1 - fog_amount) + fogcolor * fog_amount ;

}

//  Updates the transformation matrix
void TransformationMatrix(glm::mat4x4& transfmatrix, glm::mat4x4 cam, mat4 rotation){
  transfmatrix = cam * rotation;
}

/*
  Computes and sorts points on a surface that are on the left and on the right
  Used in DrawRows to draw rows to fill a triangle
*/
void ComputePolygonRows(const std::vector<Pixel>& vertexPixels,std::vector<Pixel>& leftPixels,std::vector<Pixel>& rightPixels ){

  int max = -std::numeric_limits<int>::max();
  int min = +std::numeric_limits<int>::max();

  int V = vertexPixels.size();

  for(int i = 0; i < V; i++){
    if(vertexPixels[i].y > max){
      max = vertexPixels[i].y;
        }
    if(vertexPixels[i].y < min){
      min = vertexPixels[i].y;
    }
  }

  int ROWS = max - min + 1;
  leftPixels = std::vector<Pixel> ( ROWS );
  rightPixels = std::vector<Pixel> ( ROWS );
  for( int i=0; i<ROWS; ++i )
  {
    leftPixels[i].x  = +std::numeric_limits<int>::max();
    rightPixels[i].x = -std::numeric_limits<int>::max();
  }

  std::vector<Pixel> edges;
  for(int i = 0; i < V; i++){
    std::vector<Pixel> edge;
    ComputeEdge(vertexPixels[i], vertexPixels[(i+1)%V], edge);
    edges.insert(edges.end(), edge.begin(), edge.end());
  }

  for (int i = 0; i < ROWS; i++){
    for(int j = 0; j < (int)edges.size(); j++){\
      if(edges[j].y == i+min){
        if(edges[j].x < leftPixels[i].x){
          leftPixels[i].y = i + min;
          leftPixels[i].x = edges[j].x;
          leftPixels[i].zinv = edges[j].zinv;
          leftPixels[i].pos3d =  edges[j].pos3d / edges[j].zinv;
          leftPixels[i].color = edges[j].color;
        }
        if(edges[j].x > rightPixels[i].x){
          rightPixels[i].y = i + min;
          rightPixels[i].x = edges[j].x;
          rightPixels[i].zinv = edges[j].zinv;
          rightPixels[i].pos3d = edges[j].pos3d / edges[j].zinv;
          rightPixels[i].color = edges[j].color;

      }
      }
    }
  }
}

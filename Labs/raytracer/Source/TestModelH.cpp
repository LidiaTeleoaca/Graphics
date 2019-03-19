
#include <glm/glm.hpp>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "TestModelH.h"

float ref = 0.0f;
float transp = 0.0f;
float ioref = 0.0f;
glm::vec3 current_colour = glm::vec3(0.0f, 0.0f, 0.0f);
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;

void Triangle::ComputeNormal()
{
  glm::vec3 e1 = glm::vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
  glm::vec3 e2 = glm::vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
  glm::vec3 normal3 = glm::normalize( glm::cross( e2, e1 ) );
  normal.x = normal3.x;
  normal.y = normal3.y;
  normal.z = normal3.z;
  normal.w = 1.0;
}

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Triangle>& triangles )
{
	using glm::vec3;
	using glm::vec4;



	// Defines colors:
	vec3 red(    0.75f, 0.15f, 0.15f );
	vec3 yellow( 0.75f, 0.75f, 0.15f );
	vec3 green(  0.15f, 0.75f, 0.15f );
	vec3 cyan(   0.15f, 0.75f, 0.75f );
	vec3 blue(   0.15f, 0.15f, 0.75f );
	vec3 purple( 0.75f, 0.15f, 0.75f );
	vec3 white(  0.75f, 0.75f, 0.75f );
	vec3 diamond(0.75f, 0.90f, 0.92f );
	//vec3 glass(  0.79f, 0.88f, 0.97f );

	triangles.clear();
	triangles.reserve( 50*2*3 );



	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec4 A(L,0,0,1);
	vec4 B(0,0,0,1);
	vec4 C(L,0,L,1);
	vec4 D(0,0,L,1);

	vec4 E(L,L,0,1);
	vec4 F(0,L,0,1);
	vec4 G(L,L,L,1);
	vec4 H(0,L,L,1);

	current_colour = green;
	// Floor:
	triangles.push_back( Triangle( C, B, A, current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle( C, D, B, current_colour, ref, transp, ioref) );

	current_colour = purple;
	// Left wall
	triangles.push_back( Triangle( A, E, C, current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle( C, E, G, current_colour, ref, transp, ioref) );

	current_colour = yellow;
	// Right wall
	triangles.push_back( Triangle( F, B, D, current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle( H, F, D, current_colour, ref, transp, ioref) );

	current_colour = cyan;
	// Ceiling
	triangles.push_back( Triangle( E, F, G, current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle( F, H, G, current_colour, ref, transp, ioref) );

	current_colour = white;
	// Back wall
	triangles.push_back( Triangle( G, D, C, current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle( G, H, D, current_colour, ref, transp, ioref) );


	// Load the diamond

	transp = 1.0f;
	ref = 0.5f;
	ioref = 1.8f;
	bool loop = true;

	FILE *file = fopen("diamond.obj", "r");
	if( file == NULL){
		printf("Can't open file\n");
		loop = false;
	}

	while(loop){

		//read first word
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		//if end of file, quit loop
		if (res == EOF)
			break;

		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			if (fscanf(file, "%f %f %f", &vertex.x, &vertex.y, &vertex.z ) == 3)
				temp_vertices.push_back(vertex);
		}
		else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			if(fscanf(file, "%f %f\n", &uv.x, &uv.y ) == 2)
				temp_uvs.push_back(uv);
		}
		else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			if(fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z ) == 3)
				temp_normals.push_back(normal);
		}

		else if ( strcmp( lineHeader, "f" ) == 0 ){
			//vec4 vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				loop = false;
			}
			vertexIndices.push_back(vertexIndex[0] );
			vertexIndices.push_back(vertexIndex[1] );
			vertexIndices.push_back(vertexIndex[2] );
			uvIndices    .push_back(uvIndex[0] );
			uvIndices    .push_back(uvIndex[1] );
			uvIndices    .push_back(uvIndex[2] );
			normalIndices.push_back(normalIndex[0] );
			normalIndices.push_back(normalIndex[1] );
			normalIndices.push_back(normalIndex[2] );

			float x_offset = 290.0f;
			float y_offset = 235.0f;
			float z_offset = 230.0f;
			float scale = 45.0f;

			vec3 temp_vertex = scale*temp_vertices[vertexIndex[0]];
			vec4 vertex1(temp_vertex.x + x_offset, temp_vertex.y + y_offset, temp_vertex.z + z_offset, 1.0f);
			temp_vertex = scale*temp_vertices[vertexIndex[1]];
			vec4 vertex2(temp_vertex.x + x_offset, temp_vertex.y + y_offset, temp_vertex.z + z_offset, 1.0f);
			temp_vertex = scale*temp_vertices[vertexIndex[2]];
			vec4 vertex3(temp_vertex.x + x_offset, temp_vertex.y + y_offset, temp_vertex.z + z_offset, 1.0f);

			current_colour = diamond;
			triangles.push_back( Triangle( vertex1, vertex2, vertex3, current_colour, ref, transp, ioref) );
		}
	}



	// ---------------------------------------------------------------------------
	// Front Glass

/*
	transp = 1.0f;
	ioref = 1.5f;
	ref = 0.1f;
	A = vec4(383,200,147,1);
	B = vec4(198,200,147,1);
	C = vec4(383,200,148,1);
	D = vec4(198,200,148,1);
	E = vec4(383,364,147,1);
	F = vec4(198,364,147,1);
	G = vec4(383,364,148,1);
	H = vec4(198,364,148,1);
	current_colour = white;
	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );
	//Back Glass
	A = vec4(383,200,305,1);
	B = vec4(198,200,305,1);
	C = vec4(383,200,306,1);
	D = vec4(198,200,306,1);
	E = vec4(383,364,305,1);
	F = vec4(198,364,305,1);
	G = vec4(383,364,306,1);
	H = vec4(198,364,306,1);
	current_colour = white;
	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );
	// Top glass
	A = vec4(383,364,150,1);
	B = vec4(198,364,150,1);
	C = vec4(383,364,303,1);
	D = vec4(198,364,303,1);
	E = vec4(383,365,150,1);
	F = vec4(198,365,150,1);
	G = vec4(383,365,303,1);
	H = vec4(198,365,303,1);
	current_colour = white;
	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );
	//right glass
	A = vec4(199,364,150,1);
	B = vec4(198,364,150,1);
	C = vec4(199,364,303,1);
	D = vec4(198,364,303,1);
	E = vec4(199,200,150,1);
	F = vec4(198,200,150,1);
	G = vec4(199,200,303,1);
	H = vec4(198,200,303,1);
	current_colour = white;
	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );
	//left glass
	A = vec4(383,364,150,1);
	B = vec4(382,364,150,1);
	C = vec4(383,364,303,1);
	D = vec4(382,364,303,1);
	E = vec4(383,200,150,1);
	F = vec4(382,200,150,1);
	G = vec4(383,200,303,1);
	H = vec4(382,200,303,1);
	current_colour = white;
	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );
*/

//GLASS

	transp = 1.0f;
	ioref = 1.5f;
	ref = 0.15f;
	A = vec4(383,200,147,1);
	B = vec4(198,200,147,1);
	C = vec4(383,200,306,1);
	D = vec4(198,200,306,1);

	E = vec4(383,365,147,1);
	F = vec4(198,365,147,1);
	G = vec4(383,365,306,1);
	H = vec4(198,365,306,1);
	current_colour = white;

	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );

	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );

	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );

	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );


/*
	// Inside of glass
	A = vec4(380,200,150,1);
	B = vec4(201,200,150,1);
	C = vec4(380,200,303,1);
	D = vec4(201,200,303,1);
	E = vec4(380,362,150,1);
	F = vec4(201,362,150,1);
	G = vec4(380,362,303,1);
	H = vec4(201,362,303,1);
	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );
*/
	/// ---------------------------------------------------------------------------
	// Tall block
	ioref = 0.0f;
	transp = 0.0f;
	ref = 0.8f;
	A = vec4(383,0,147,1);
	B = vec4(198,0,147,1);
	C = vec4(383,0,306,1);
	D = vec4(198,0,306,1);

	E = vec4(383,200,147,1);
	F = vec4(198,200,147,1);
	G = vec4(383,200,306,1);
	H = vec4(198,200,306,1);

	current_colour = blue;

	// FRONT
	triangles.push_back( Triangle(E,B,A,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,current_colour, ref, transp, ioref) );

	// RIGHT
	triangles.push_back( Triangle(F,D,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,current_colour, ref, transp, ioref) );

	// BACK
	triangles.push_back( Triangle(H,C,D,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,current_colour, ref, transp, ioref) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,current_colour, ref, transp, ioref) );

	// TOP
	triangles.push_back( Triangle(G,F,E,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,current_colour, ref, transp, ioref) );

	// BOTTOM
	triangles.push_back( Triangle(A,C,B,current_colour, ref, transp, ioref) );
	triangles.push_back( Triangle(B,C,D,current_colour, ref, transp, ioref) );
/*
	// Small cube
	ref = 0.0f;
	ioref = 0.0f;
	transp = 0.0f;
	A = vec4(200,0,114,1);
	B = vec4(130,0, 65,1);
	C = vec4(150,0,272,1);
	D = vec4( 82,0,225,1);
	E = vec4(200,100,114,1);
	F = vec4(130,100, 65,1);
	G = vec4(150,100,272,1);
	H = vec4( 82,100,225,1);
	// FRONT
	triangles.push_back( Triangle(E,B,A,red, ref, transp, ioref) );
	triangles.push_back( Triangle(E,F,B,red, ref, transp, ioref) );
	// RIGHT
	triangles.push_back( Triangle(F,D,B,red, ref, transp, ioref) );
	triangles.push_back( Triangle(F,H,D,red, ref, transp, ioref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,red, ref, transp, ioref) );
	triangles.push_back( Triangle(H,G,C,red, ref, transp, ioref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,red, ref, transp, ioref) );
	triangles.push_back( Triangle(E,A,C,red, ref, transp, ioref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,red, ref, transp, ioref) );
	triangles.push_back( Triangle(G,H,F,red, ref, transp, ioref) );
	// BOTTOM
	triangles.push_back( Triangle(A,C,B,red, ref, transp, ioref) );
	triangles.push_back( Triangle(B,C,D,red, ref, transp, ioref) );
//original triangles
	// ---------------------------------------------------------------------------
	// Short block
	ref = 1.0f;
	A = vec4(290,0,114,1);
	B = vec4(130,0, 65,1);
	C = vec4(240,0,272,1);
	D = vec4( 82,0,225,1);
	E = vec4(290,165,114,1);
	F = vec4(130,165, 65,1);
	G = vec4(240,165,272,1);
	H = vec4( 82,165,225,1);
	// Front
	triangles.push_back( Triangle(E,B,A,red, ref) );
	triangles.push_back( Triangle(E,F,B,red, ref) );
	// Front
	triangles.push_back( Triangle(F,D,B,red, ref) );
	triangles.push_back( Triangle(F,H,D,red, ref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,red, ref) );
	triangles.push_back( Triangle(H,G,C,red, ref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,red, ref) );
	triangles.push_back( Triangle(E,A,C,red, ref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,red, ref) );
	triangles.push_back( Triangle(G,H,F,red, ref) );
	// ---------------------------------------------------------------------------
	// Tall block
	A = vec4(423,0,247,1);
	B = vec4(265,0,296,1);
	C = vec4(472,0,406,1);
	D = vec4(314,0,456,1);
	E = vec4(423,330,247,1);
	F = vec4(265,330,296,1);
	G = vec4(472,330,406,1);
	H = vec4(314,330,456,1);
	// Front
	triangles.push_back( Triangle(E,B,A,blue, ref) );
	triangles.push_back( Triangle(E,F,B,blue, ref) );
	// Front
	triangles.push_back( Triangle(F,D,B,blue, ref) );
	triangles.push_back( Triangle(F,H,D,blue, ref) );
	// BACK
	triangles.push_back( Triangle(H,C,D,blue, ref) );
	triangles.push_back( Triangle(H,G,C,blue, ref) );
	// LEFT
	triangles.push_back( Triangle(G,E,C,blue, ref) );
	triangles.push_back( Triangle(E,A,C,blue, ref) );
	// TOP
	triangles.push_back( Triangle(G,F,E,blue, ref) );
	triangles.push_back( Triangle(G,H,F,blue, ref) );
*/

	// ----------------------------------------------
	// Scale to the volume [-1,1]^3

	for( size_t i=0; i<triangles.size(); ++i )
	{
		triangles[i].v0 *= 2/L;
		triangles[i].v1 *= 2/L;
		triangles[i].v2 *= 2/L;

		triangles[i].v0 -= vec4(1,1,1,1);
		triangles[i].v1 -= vec4(1,1,1,1);
		triangles[i].v2 -= vec4(1,1,1,1);

		triangles[i].v0.x *= -1;
		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].v0.w = 1.0;
		triangles[i].v1.w = 1.0;
		triangles[i].v2.w = 1.0;

		triangles[i].ComputeNormal();
	}
}

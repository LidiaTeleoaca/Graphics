#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>
#include <stdio.h>


// Used to describe a triangular surface:
class Triangle
{
public:
	glm::vec4 v0;
	glm::vec4 v1;
	glm::vec4 v2;
	glm::vec4 normal;
	glm::vec3 color;
	float reflective; // how reflective a triangle is. 0 = not reflective, 1 = mirror
	float transparent; // how transparent an object is. 0 = not transp. 1 = 100% transp
	float ior; //indice of refraction. 0 = no refraction; 1 = air, else: index of refraction


	Triangle( glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec3 color, float reflective, float transparent, float ior )
		: v0(v0), v1(v1), v2(v2), color(color), reflective(reflective), transparent(transparent), ior(ior)
	{
		ComputeNormal();
	}

	Triangle( glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec4 normal, glm::vec3 color, float reflective, float transparent, float ior )
		: v0(v0), v1(v1), v2(v2), normal(normal), color(color), reflective(reflective), transparent(transparent), ior(ior)
	{
	}

	void ComputeNormal();

};

void LoadTestModel( std::vector<Triangle>& triangles );


#endif

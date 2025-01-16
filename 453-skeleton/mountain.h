#pragma once

#include "Geometry.h"
#include <glm/gtx/transform.hpp>
#include "Texture.h"
#include "config.h"
#include "Vertex.h"
#include "SimplexNoise.h"


class mountain {
public:
	mountain(std::string _name, std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation)
	{
		//elevate();
	}
	CPU_Geometry m_cpu_geom;    // We dont really need atm
	GPU_Geometry m_gpu_geom;
	glm::mat4 m_model;
	GLsizei m_size;

	float ridge(float h, float offset);
	float ridgedMF(float x, float y, SimplexNoise noise);
	float getDistance(float x1, float y1, float x2, float y2);
	void computeNormals(
		const std::vector<unsigned int>& indices,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec3>& verts);
	void elevate();
	void updateConfig(config config);

	std::string name;

	Texture texture;
	bool render;
	int size;
	config _config;
	

private:

};


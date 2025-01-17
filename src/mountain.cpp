#include "mountain.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <cmath>
#include <algorithm> 
#include <chrono>
#include <thread>
#include <random>

float mountain::ridge(float h, float offset)
{
	h = offset - std::fabs(h);

	if (h < 0.5f) {
		// 4 * h^3
		return 4.0f * h * h * h;
	}
	else {
		// (h - 1) * (2h - 2)^2 + 1
		float term = (2.0f * h - 2.0f);
		return (h - 1.0f) * term * term + 1.0f;
	}
}

float mountain::ridgedMF(float x, float y, SimplexNoise noise)
{
	float sum = 0.0f;
	float amp = 0.5f;
	float prev = 1.0f;
	float freq = _config.frequency;

	for (int i = 0; i < _config.octaves; i++) {
		float h = noise.noise2D(x * freq, y * freq);
		float n = ridge(h, _config.ridgeOffset);

		sum += n * amp * prev;
		prev = n;
		freq *= _config.lacunarity;
		amp *= _config.gain;
	}

	return sum;
}

float mountain::getDistance(float x1, float y1, float x2, float y2)
{
	float dx = std::fabs(x1 - x2);
	float dy = std::fabs(y1 - y2);
	return std::sqrt(dx * dx + dy * dy);
}

void mountain::computeNormals(
	const std::vector<unsigned int>& indices,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec3>& verts)
{
	// Reset normals
	for (auto& v : normals) {
		v.x = 0.0f;
		v.y = 0.0f;
		v.z = 0.0f;
	}

	for (size_t i = 0; i < indices.size(); i += 3) {
		unsigned int i0 = indices[i];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		glm::vec3& v0 = verts[i0];
		glm::vec3& v1 = verts[i1];
		glm::vec3& v2 = verts[i2];

		// Two edges of the triangle
		float ux = v1.x - v0.x;
		float uy = v1.y - v0.y;
		float uz = v1.z - v0.z;

		float vx = v2.x - v0.x;
		float vy = v2.y - v0.y;
		float vz = v2.z - v0.z;

		// Cross product to get face normal
		float nx = (uy * vz) - (uz * vy);
		float ny = (uz * vx) - (ux * vz);
		float nz = (ux * vy) - (uy * vx);

		// Accumulate
		normals[i0].x += nx;  normals[i0].y += ny;  normals[i0].z += nz;
		normals[i1].x += nx;  normals[i1].y += ny;  normals[i1].z += nz;
		normals[i2].x += nx;  normals[i2].y += ny;  normals[i2].z += nz;
	}

	 //Normalize the accumulated normals
	for (auto& v : normals) {
		float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (length > 1e-6f) {
			v.x /= length;
			v.y /= length;
			v.z /= length;
		}
	}
}

void mountain::elevate()
{
 	//start time
	auto start = std::chrono::high_resolution_clock::now();
 
	SimplexNoise noise(_config.seed);
	int width = _config.width;
	int height = _config.height;
	int subdivisions = _config.subdivisions;

	// (subdivisions+1) x (subdivisions+1) grid
	// store all vertices in a single vector
	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> normals;
	verts.resize((subdivisions + 1) * (subdivisions + 1));
	normals.resize((subdivisions + 1) * (subdivisions + 1));

	std::vector<glm::vec2> texcoords;
	texcoords.resize((subdivisions + 1) * (subdivisions + 1));

	// Generate vertex positions, heights, and (optionally) texcoords
	for (int row = 0; row <= subdivisions; row++) {
		for (int col = 0; col <= subdivisions; col++) {
			// Index of the current vertex in the array
			int index = row * (subdivisions + 1) + col;

			float posX = col * (width / (float)subdivisions) - (width / 2.0f);
			float posZ = row * (height / (float)subdivisions) - (height / 2.0f);

			float x = (posX + (width / 2.0f)) / (float)width;
			float y = (-(posZ)+(height / 2.0f)) / (float)height;

			float h = ridgedMF(x, y, noise);

			float distance = getDistance(x, y, 0.5f, 0.5f);
			float falloff = std::min(1.0f - (distance / 0.5f), 1.0f);
			if (falloff < 0.0f) {
				falloff = 0.0f;
			}

			float finalHeight = h * falloff * 15.0f;

			verts[index].x = posX;
			if (finalHeight < 0) verts[index].y = -finalHeight;
			if (finalHeight >= 0) verts[index].y = finalHeight;
			verts[index].z = posZ;

			// Simple UV mapping [0..1]
			texcoords[index] = glm::vec2(
				col / (float)subdivisions,
				row / (float)subdivisions
			);
		}
	}

	//time after first loop
	auto afterFirstLoop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedFirstLoop = afterFirstLoop - start;
	std::cout << "First loop time: " << elapsedFirstLoop.count() << " s\n";

	// Generate indices for a standard grid of triangles
	std::vector<unsigned int> indices;
	indices.reserve(subdivisions * subdivisions * 6);

	for (int row = 0; row < subdivisions; row++) {
		for (int col = 0; col < subdivisions; col++) {
			int i0 = row * (subdivisions + 1) + col;
			int i1 = row * (subdivisions + 1) + (col + 1);
			int i2 = (row + 1) * (subdivisions + 1) + col;
			int i3 = (row + 1) * (subdivisions + 1) + (col + 1);

			// Two triangles per quad
			// Triangle 1
			indices.push_back(i0);
			indices.push_back(i1);
			indices.push_back(i2);

			// Triangle 2
			indices.push_back(i1);
			indices.push_back(i3);
			indices.push_back(i2);
		}
	}

	//time after second loop
	auto afterSecondLoop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedSecondLoop = afterSecondLoop - afterFirstLoop;
	std::cout << "Second loop time: " << elapsedSecondLoop.count() << " s\n";

	// Compute normals for the entire mesh
	computeNormals(indices, normals, verts);
	std::vector<glm::vec3> finalVerts;
	std::vector<glm::vec3> finalNormals;
	std::vector<glm::vec2> finalTexcoords;
	finalVerts.resize(indices.size());
	finalNormals.resize(indices.size());
	finalTexcoords.resize(indices.size());

	for (int i = 0; i < indices.size(); i++) {
		finalVerts[i] = verts[indices[i]];
		finalNormals[i] = normals[indices[i]];
		finalTexcoords[i] = texcoords[indices[i]];
	}

	//third loop time
	auto thirdLoop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedThirdLoop = thirdLoop - afterSecondLoop;
	std::cout << "Third loop time: " << elapsedThirdLoop.count() << " s\n";

	m_cpu_geom.verts = finalVerts;
	m_cpu_geom.normals = finalNormals;

	//m_gpu_geom.bind();
	m_gpu_geom.setVerts(m_cpu_geom.verts);
	m_gpu_geom.setNormals(m_cpu_geom.normals);

	//time after fourth loop
	auto fourthLoop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedFourthLoop = fourthLoop - thirdLoop;
	std::cout << "Fourth loop time: " << elapsedFourthLoop.count() << " s\n";

	m_gpu_geom.setTexCoords(finalTexcoords);

	//size
	m_size = m_cpu_geom.verts.size();

 	//end time
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "Normal time: " << elapsed.count() << " s\n";
}

void mountain::updateConfig(config _newConfig)
{
	this->_config = _newConfig;
	elevate();
}

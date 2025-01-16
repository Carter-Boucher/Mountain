#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:
	Camera(float t, float p, float r);

	glm::mat4 getView();
	glm::vec3 getPos();

	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);

	// New function to set a different look-at target
	glm::vec3 getTarget();
	void setTarget(const glm::vec3& newTarget);


	glm::vec3 target;
private:
	float theta;
	float phi;
	float radius;

	// A new member variable that indicates where the camera should look.
	
};

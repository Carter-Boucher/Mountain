#include "Camera.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(float t, float p, float r)
	: theta(t), phi(p), radius(r), target(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

glm::mat4 Camera::getView()
{
	// Calculate the camera’s position in spherical coordinates
	glm::vec3 eye = radius * glm::vec3(
		std::cos(theta) * std::sin(phi),
		std::sin(theta),
		std::cos(theta) * std::cos(phi)
	);

	// Use the 'target' member variable to look at a new location
	glm::vec3 at = target;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	// Return the lookAt matrix
	return glm::lookAt(eye, at, up);
}

glm::vec3 Camera::getPos()
{
	return radius * glm::vec3(
		std::cos(theta) * std::sin(phi),
		std::sin(theta),
		std::cos(theta) * std::cos(phi)
	);
}

void Camera::incrementTheta(float dt)
{
	if (theta + (dt / 100.0f) < M_PI_2 && theta + (dt / 100.0f) > -M_PI_2) {
		theta += dt / 100.0f;
	}
}

void Camera::incrementPhi(float dp)
{
	phi -= dp / 100.0f;
	if (phi > 2.0 * M_PI) {
		phi -= 2.0 * M_PI;
	}
	else if (phi < 0.0f) {
		phi += 2.0 * M_PI;
	}
}

void Camera::incrementR(float dr)
{
	radius -= dr;
}

glm::vec3 Camera::getTarget() {
	return target;
}

// New function to set a different look-at target
void Camera::setTarget(const glm::vec3& newTarget)
{
	target = newTarget;
}

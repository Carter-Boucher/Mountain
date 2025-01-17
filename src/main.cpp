//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <filesystem> // C++17

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "mountain.h"

#include "config.h"

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {
public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, leftMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {}
	virtual void mouseButtonCallback(int button, int action, int mods) override {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)            rightMouseDown = true;
			else if (action == GLFW_RELEASE)     rightMouseDown = false;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS)            leftMouseDown = true;
			else if (action == GLFW_RELEASE)     leftMouseDown = false;
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) override {
		// right mouse rotation
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}

		// left mouse translation
		if (leftMouseDown) {
			glm::vec3 target = camera.getTarget();
			target.x -= (xpos - mouseOldX) * 0.1f;
			target.y += (ypos - mouseOldY) * 0.1f;
			camera.setTarget(target);
		}

		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) override {
		camera.incrementR(yoffset * 10.0);
	}
	virtual void windowSizeCallback(int width, int height) override {
		// The base CallbackInterface::windowSizeCallback calls glViewport.
		CallbackInterface::windowSizeCallback(width, height);
		aspect = float(width) / float(height);
	}

	void viewPipeline(ShaderProgram& sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		// Send to shader
		glUniformMatrix4fv(glGetUniformLocation(sp, "M"), 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(glGetUniformLocation(sp, "V"), 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix4fv(glGetUniformLocation(sp, "P"), 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;
private:
	bool rightMouseDown;
	bool leftMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "Moutnain Render");

	//GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// MOUNTAIN
	// The constructor presumably loads geometry or sets up VAOs, etc.
	mountain mountain1("mountain1", "textures/rock.jpg", GL_LINEAR);
	config currentConfig = loadConfig("config.txt");
	mountain1.updateConfig(currentConfig);

	/*mountain Mountain2("mountain2", "textures/rock.png", GL_LINEAR);
	Mountain2.updateConfig(currentConfig);*/

	std::filesystem::file_time_type lastWriteTime;
	try {
		lastWriteTime = std::filesystem::last_write_time("config.txt");
	}
	catch (std::filesystem::filesystem_error& e) {
		std::cerr << "Error getting file time: " << e.what() << std::endl;
	}
	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();
		try {
			auto newWriteTime = std::filesystem::last_write_time("config.txt");
			if (newWriteTime != lastWriteTime) {
				std::cout << "Config file changed. Reloading...\n";
				currentConfig = loadConfig("config.txt");
				lastWriteTime = newWriteTime;

				 mountain1.updateConfig(currentConfig);
			}
		}
		catch (std::filesystem::filesystem_error& e) {
			std::cerr << "Error checking file time: " << e.what() << std::endl;
		}

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(glm::vec3(10.0f, 10.0f, 3.0f)));
		glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(a4->camera.getPos()));
		glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

		shader.use();
		a4->viewPipeline(shader);

		mountain1.m_gpu_geom.bind();
		mountain1.texture.bind();
		glPointSize(currentConfig.dotSize);
		if (currentConfig.type == 0) glDrawArrays(GL_POINTS, 0, mountain1.m_size);
		if (currentConfig.type == 1) glDrawArrays(GL_TRIANGLES, 0, mountain1.m_size);
		mountain1.texture.unbind();
		
		mountain1.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for anything else
		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}

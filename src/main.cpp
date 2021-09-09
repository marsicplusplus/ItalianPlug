#include "glad/glad.h"

 #include "GLFW/glfw3.h"

#include <iostream>

#define CHECK_ERROR(COND, MESSAGE, RET) if(!(COND)){\
	std::cerr << (MESSAGE);\
	return (RET);\
}

void processInput(GLFWwindow *window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	GLFWwindow *window;

	CHECK_ERROR(glfwInit(), "ERROR::Renderer::initSystems > Cannot initialize glfw\n", false);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	CHECK_ERROR(window = glfwCreateWindow(800, 600, "Multimedia Retrieval!", NULL, NULL), "ERROR::Renderer::initSystems > could not create GLFW3 window\n", false);

	glfwMakeContextCurrent(window);

	while(!glfwWindowShouldClose(window)){
		processInput(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}



#include "renderer.hpp"
#include <iostream>
#include "mesh.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.hpp"
#include "input_handler.hpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "camera.hpp"
#include <memory>

#define CHECK_ERROR(COND, MESSAGE, RET) if(!(COND)){\
	std::cerr << (MESSAGE);\
	return (RET);\
}

Renderer::~Renderer() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	MeshMap::Instance()->destroy();
	OptionsMap::Instance()->destroy();
}

bool Renderer::initSystems(){
	CHECK_ERROR(glfwInit(), "ERROR::Renderer::initSystems > Cannot initialize glfw\n", false)

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	CHECK_ERROR(window = glfwCreateWindow(wWidth, wHeight, title.c_str(), NULL, NULL), "ERROR::Renderer::initSystems > could not create GLFW3 window\n", false)

	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwSetScrollCallback(window, scrollCallback);

	CHECK_ERROR(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize GLAD", false);

	glEnable(GL_DEPTH_TEST);  

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// (optional) set browser properties
	fileDialog.SetTitle("Choose a Mesh");
	fileDialog.SetTypeFilters({ ".off", ".ply" });

	OptionsMap::Instance()->setOption(DRAW_MODE, POINT_CLOUD);

	return true;
}

void Renderer::start() {
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	float LOW_LIMIT = 1.0f/60.0f;          // Keep At/Below 60fps
	float HIGH_LIMIT = 1.0f/10.0f;            // Keep At/Above 10fps

	float lastTime = glfwGetTime();

	glm::mat4 proj = glm::perspective(camera.getFOV(), (float)wWidth/(float)wHeight, 0.1f, 100.0f);

	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		InputHandler::Instance()->setMouseState(xpos, ypos);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, wWidth, wHeight);

		float currentTime = glfwGetTime();
		float deltaTime = ( currentTime - lastTime ) / 1000.0f;
		if ( deltaTime < LOW_LIMIT )
			deltaTime = LOW_LIMIT;
		else if ( deltaTime > HIGH_LIMIT )
			deltaTime = HIGH_LIMIT;
		lastTime = currentTime;

		if(mesh != nullptr){
			if(!fileDialog.IsOpened()){
				camera.update(deltaTime);
				MouseState ms = InputHandler::Instance()->getMouseState();
				if(ms.moved) mesh->mouseMoved(ms.dx, ms.dy);
				mesh->update(deltaTime);
			}
			glm::mat4 projView = proj*camera.getViewMatrix();
			mesh->draw(projView);
			if(OptionsMap::Instance()->getOption(DRAW_MODE) == SHADED_MESH_WIREFRAME){
				OptionsMap::Instance()->setOption(DRAW_MODE, WIREFRAME);
				mesh->draw(projView);
				OptionsMap::Instance()->setOption(DRAW_MODE, SHADED_MESH_WIREFRAME);
			}
		}
		renderGUI();

		glfwSwapBuffers(window);
	}
}

void Renderer::renderGUI(){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	{
		ImGui::NewFrame();

		// render your GUI
		ImGui::Begin("Change Mesh");
		if(ImGui::Button("File")){
			fileDialog.Open();
		}
		if(ImGui::Button("Next Drawing Mode")){
			int mode = (OptionsMap::Instance()->getOption(DRAW_MODE));
			OptionsMap::Instance()->setOption(DRAW_MODE, (mode + 1) % DRAW_MODES);
		}
		if(ImGui::Button("Exit")){
			glfwSetWindowShouldClose(window, true);
		}
		ImGui::End();

		fileDialog.Display();
		if(fileDialog.HasSelected()) {
			mesh = MeshMap::Instance()->getMesh(fileDialog.GetSelected().string());
			camera.setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
			fileDialog.ClearSelected();
		}
	}
	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**************** GLFW Callbacks ****************/

void Renderer::resizeWindow(int w, int h){
	wWidth = w;
	wHeight = h;
}

void Renderer::windowSizeCallback(GLFWwindow* window, int width, int height) {
	Renderer* obj = (Renderer*)glfwGetWindowUserPointer(window);
	obj->resizeWindow(width, height);
}

void Renderer::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

void Renderer::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT){
		InputHandler::Instance()->setKeyValue(MOUSE_RIGHT, (action == GLFW_PRESS) ? true : false);
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT){
		InputHandler::Instance()->setKeyValue(MOUSE_LEFT, (action == GLFW_PRESS) ? true : false);
	}
}

void Renderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
	InputHandler::Instance()->scrollState(yoffset);
}

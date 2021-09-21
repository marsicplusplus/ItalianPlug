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
#include "spdlog/spdlog.h"
#include <memory>

#define CHECK_ERROR(COND, MESSAGE, RET) if(!(COND)){\
	spdlog::error((MESSAGE));\
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

void GLAPIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	spdlog::error("Debug message ({}): {}", id, message);
	
	std::stringstream ss;
	switch (source) {
	case GL_DEBUG_SOURCE_API:             ss << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ss << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: ss << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     ss << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     ss << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           ss << "Source: Other"; break;
	} ss << std::endl;

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               ss << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ss << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         ss << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         ss << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              ss << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          ss << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           ss << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               ss << "Type: Other"; break;
	} ss << std::endl;

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         ss << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       ss << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          ss << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: ss << "Severity: notification"; break;
	} ss << std::endl;
	ss << std::endl;
	spdlog::error(ss.str());
}

bool Renderer::initSystems(){
	CHECK_ERROR(glfwInit(), "ERROR::Renderer::initSystems > Cannot initialize glfw", false)

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	CHECK_ERROR(window = glfwCreateWindow(wWidth, wHeight, title.c_str(), NULL, NULL), "ERROR::Renderer::initSystems > could not create GLFW3 window", false)

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
	setupImGuiStyle();

	materialDiffuse = glm::vec3(0.8f, 0.4f, 0.4f);
	// (optional) set browser properties
	fileDialog.SetTitle("Choose a Mesh");
	fileDialog.SetTypeFilters({ ".off", ".ply" });

	OptionsMap::Instance()->setOption(DRAW_MODE, SHADED_MESH);
	glPointSize(3.0f);

	// Setup Debugging
	//int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	//if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		//glEnable(GL_DEBUG_OUTPUT);
		//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//glDebugMessageCallback(glDebugOutput, nullptr);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	//}

	return true;
}

void Renderer::setMesh(std::string path){
	mesh = MeshMap::Instance()->getMesh(path);
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
				mesh->update(deltaTime);
			}
			glm::mat4 projView = proj*camera.getViewMatrix();
			mesh->draw(projView, materialDiffuse, camera.getPosition());
			if(OptionsMap::Instance()->getOption(DRAW_MODE) == SHADED_MESH_WIREFRAME){
				OptionsMap::Instance()->setOption(DRAW_MODE, WIREFRAME);
				mesh->draw(projView, materialDiffuse, camera.getPosition());
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
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(.0f, .0f));
	ImGui::SetNextWindowSize(ImVec2(wWidth / 5, wHeight));
	{
		ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		// render your GUI
    	if (ImGui::CollapsingHeader("File")){
			if(ImGui::Button("Load Mesh")){
				fileDialog.Open();
			}
			if(ImGui::Button("Exit")){
				glfwSetWindowShouldClose(window, true);
			}
			fileDialog.Display();
			if(fileDialog.HasSelected()) {
				mesh = MeshMap::Instance()->getMesh(fileDialog.GetSelected().string());
				camera.setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
				fileDialog.ClearSelected();
			}
		}
    	if (ImGui::CollapsingHeader("Mesh")){
			ImGui::Text((mesh) ? mesh->getPath().c_str() : "Load a mesh!");
			ImGui::Text("# of vertices: %d", (mesh) ? mesh->countVertices() : 0);
			ImGui::Text("# of faces: %d", (mesh) ? mesh->countFaces() : 0);
			ImGui::Separator();
			const char* items[] = {"Wireframe", "Point Cloud", "Shaded Mesh", "Shaded Mesh + Wireframe"};
			static int currentIdx = OptionsMap::Instance()->getOption(DRAW_MODE);
			const char* preview = items[currentIdx];
			if (ImGui::BeginCombo("## empty", preview, 0)) {
				for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
					const bool isSelected = (currentIdx == n);
					if (ImGui::Selectable(items[n], isSelected)){
						currentIdx = n;
						OptionsMap::Instance()->setOption(DRAW_MODE, n);
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if(ImGui::Button("Reset View")){
				mesh->resetTransformations();
				camera.setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
			}
			ImGui::Text("Colour Picker");
			ImGui::ColorEdit3("", &materialDiffuse[0]);
		}
		ImGui::End();
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
	auto inputHandler = InputHandler::Instance();
	if (key == GLFW_KEY_W)
		inputHandler->setKeyValue(KEYBOARD_W, (action == GLFW_PRESS) ? true : false);
	if (key == GLFW_KEY_A)
		inputHandler->setKeyValue(KEYBOARD_A, (action == GLFW_PRESS) ? true : false);
	if (key == GLFW_KEY_S)
		inputHandler->setKeyValue(KEYBOARD_S, (action == GLFW_PRESS) ? true : false);
	if (key == GLFW_KEY_D)
		inputHandler->setKeyValue(KEYBOARD_D, (action == GLFW_PRESS) ? true : false);
	if (key == GLFW_KEY_T)
		inputHandler->setKeyValue(KEYBOARD_T, (action == GLFW_PRESS) ? true : false);
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

void Renderer::setupImGuiStyle()
{
	ImGuiIO& imGuiIO = ImGui::GetIO();
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

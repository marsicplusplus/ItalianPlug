#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "mesh.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imfilebrowser.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <string>
#include <memory>

class Renderer {
	public:
		int wWidth;
		int wHeight;
		std::string title;

	public:
		Renderer(int w, int h, std::string title) : wWidth(w), wHeight(h), title(title), mesh(nullptr) {}
		~Renderer();
		bool initSystems();
		void start();
		void resizeWindow(int w, int h);

	private:
		GLFWwindow *window;
		ImGui::FileBrowser fileDialog;
		std::unique_ptr<Mesh> mesh;

		void renderGUI();

		static void windowSizeCallback(GLFWwindow* window, int width, int height);
		static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif

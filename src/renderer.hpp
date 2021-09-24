#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "mesh.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imfilebrowser.h"
#include <string>
#include <memory>
#include "camera.hpp"
#include "mesh_map.hpp"
#include "unit_cube.hpp"

class Renderer {
	public:
		int wWidth;
		int wHeight;
		std::string title;

	public:
		Renderer(int w, int h, std::string title) : wWidth(w), wHeight(h), title(title), mesh(nullptr), camera({0.0f, 0.0f, 1.5f}, {0.0f, 1.0f, 0.0f}, 45.0f, -90, 0, 0) {}
		~Renderer();
		bool initSystems();
		void start();
		void resizeWindow(int w, int h);
		void setMesh(std::string path);

	private:
		GLFWwindow *window;
		ImGui::FileBrowser fileDialog;
		MeshPtr mesh;
		Camera camera;
		glm::vec3 materialDiffuse;
		bool displayUnitCube;


		void renderGUI();
		void setupImGuiStyle();

		static void windowSizeCallback(GLFWwindow* window, int width, int height);
		static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

};

#endif

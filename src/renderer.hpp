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
#include <future>
#include <thread>

class Renderer {


	public:
		Renderer(int w, int h, std::string title) : m_wWidth(w), m_wHeight(h), m_title(title), m_mesh(nullptr), m_camera({0.0f, 0.0f, 1.5f}, {0.0f, 1.0f, 0.0f}, 45.0f, -90, 0, 0), m_gui(true), m_featuresPresent(false), m_annPresent(false) {}
		~Renderer();
		bool initSystems();
		void start();
		void resizeWindow(int w, int h);
		void setMesh(std::string path);
		void renderGUI();
		void setupImGuiStyle();
		void takeScreenshots(std::filesystem::path dbPath);
		void loadScreenshot(std::filesystem::path shapePath);

		static void windowSizeCallback(GLFWwindow* window, int width, int height);
		static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

public:
	int m_wWidth;
	int m_wHeight;
	std::string m_title;

private:
	GLFWwindow* m_window;
	ImGui::FileBrowser m_fileDialog;
	ImGui::FileBrowser m_folderDialog;
	std::filesystem::path m_dbPath;

	MeshPtr m_mesh;
	Camera m_camera;
	glm::vec3 m_meshMaterialDiffuse;
	glm::vec3 m_convexHullMaterialDiffuse;
	bool m_renderMesh;
	bool m_renderConvexHull;
	bool m_renderUnitCube;
	bool m_gui;
	bool m_takeScrenshot;
	bool m_featuresPresent;
	bool m_annPresent;
	std::unordered_map<std::string, std::tuple<GLuint, int, int>> meshToTexture;
	std::future<MeshPtr> m_normalizing_future;
	bool m_normalized = false;
	std::future<void> m_retrieval_future;
	bool m_retrieved = false;
	std::string m_retrieval_text;
	int m_numShapes = 0;

	inline static void HelpMarker(const char* desc) {
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
};

#endif

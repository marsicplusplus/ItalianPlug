#include "renderer.hpp"
#include <iostream>
#include "mesh.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.hpp"
#include "shader_map.hpp"
#include "input_handler.hpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include "shape_retriever.hpp"
#include "camera.hpp"
#include "tsne_runner.hpp"
#include <memory>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CHECK_ERROR(COND, MESSAGE, RET) if(!(COND)){\
	std::cerr << (MESSAGE);\
	return (RET);\
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint * out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}


Renderer::~Renderer() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_window);
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

	std::cerr << "---------------" << std::endl;
	std::cerr << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
	} std::cerr << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
	} std::cerr << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
	} std::cerr << std::endl;
	std::cerr << std::endl;
}

bool Renderer::initSystems(){
	CHECK_ERROR(glfwInit(), "ERROR::Renderer::initSystems > Cannot initialize glfw\n", false)

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	CHECK_ERROR(m_window = glfwCreateWindow(m_wWidth, m_wHeight, m_title.c_str(), NULL, NULL), "ERROR::Renderer::initSystems > could not create GLFW3 window\n", false)

	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetKeyCallback(m_window, keyboardCallback);
	glfwSetMouseButtonCallback(m_window, mouseCallback);
	glfwSetMouseButtonCallback(m_window, mouseCallback);
	glfwSetWindowSizeCallback(m_window, windowSizeCallback);
	glfwSetScrollCallback(m_window, scrollCallback);

	CHECK_ERROR(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize GLAD", false);

	glEnable(GL_DEPTH_TEST);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
	// Setup Dear ImGui style
	setupImGuiStyle();

	m_meshMaterialDiffuse = glm::vec3(0.8f, 0.4f, 0.4f);
	m_convexHullMaterialDiffuse = glm::vec3(0.8f, 0.4f, 0.4f);
	// (optional) set browser properties
	m_fileDialog.SetTitle("Choose a Mesh");
	m_fileDialog.SetTypeFilters({ ".off", ".ply" });

	m_folderDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory);
	m_folderDialog.SetTitle("Choose database folder");


	OptionsMap::Instance()->setOption(DRAW_MODE, SHADED_MESH);
	glPointSize(3.0f);

	m_renderMesh = true;
	m_renderConvexHull = false;
	m_renderUnitCube = false;

	// Setup Debugging
	//int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	//if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		//glEnable(GL_DEBUG_OUTPUT);
		//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//glDebugMessageCallback(glDebugOutput, nullptr);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	//}
	auto s = ShaderMap::Instance()->getShader(SHADER_BASE);
	s->loadShader("shaders/basic_vertex.glsl", GL_VERTEX_SHADER);
	s->loadShader("shaders/basic_fragment.glsl", GL_FRAGMENT_SHADER);
	s->compileShaders();
	s = ShaderMap::Instance()->getShader(SHADER_EDGE);
	s->loadShader("shaders/basic_vertex.glsl", GL_VERTEX_SHADER);
	s->loadShader("shaders/edge_fragment.glsl", GL_FRAGMENT_SHADER);
	s->compileShaders();

	return true;
}

void Renderer::setMesh(std::string path){
	m_mesh = MeshMap::Instance()->getMesh(path);
	m_mesh->prepare();
}

void Renderer::start() {
	UnitCube unitCube;
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	float LOW_LIMIT = 1.0f/60.0f;          // Keep At/Below 60fps
	float HIGH_LIMIT = 1.0f/10.0f;            // Keep At/Above 10fps

	float lastTime = glfwGetTime();

	glm::mat4 proj = glm::perspective(m_camera.getFOV(), (float)m_wWidth/(float)m_wHeight, 0.1f, 100.0f);
	while(!glfwWindowShouldClose(m_window)){
		glfwPollEvents();
		double xpos, ypos;
		glfwGetCursorPos(m_window, &xpos, &ypos);
		InputHandler::Instance()->setMouseState(xpos, ypos);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, m_wWidth, m_wHeight);

		if(InputHandler::Instance()->isKeyDown(KEYBOARD_T)){
			m_gui = true;
		}
		if(InputHandler::Instance()->isKeyDown(KEYBOARD_N)){
			m_gui = false;
		}

		float currentTime = glfwGetTime();
		float deltaTime = ( currentTime - lastTime ) / 1000.0f;
		if ( deltaTime < LOW_LIMIT )
			deltaTime = LOW_LIMIT;
		else if ( deltaTime > HIGH_LIMIT )
			deltaTime = HIGH_LIMIT;
		lastTime = currentTime;

		glm::mat4 projView = proj * m_camera.getViewMatrix();

		if (m_renderUnitCube) {
			unitCube.draw(projView);
		}

		if(m_mesh != nullptr){
			if(!m_fileDialog.IsOpened() && !m_plotTSE){
				m_camera.update(deltaTime);
				m_mesh->update(deltaTime);
			}

			if (m_renderMesh) {
				m_mesh->draw(projView, m_meshMaterialDiffuse, m_camera.getPosition());
				if (OptionsMap::Instance()->getOption(DRAW_MODE) == SHADED_MESH_WIREFRAME) {
					OptionsMap::Instance()->setOption(DRAW_MODE, WIREFRAME);
					m_mesh->draw(projView, m_meshMaterialDiffuse, m_camera.getPosition());
					OptionsMap::Instance()->setOption(DRAW_MODE, SHADED_MESH_WIREFRAME);
				}
			}

			if (m_renderConvexHull) {
				m_mesh->getConvexHull()->draw(projView, m_convexHullMaterialDiffuse, m_camera.getPosition());
			}
		}
		if(m_gui){
			renderGUI();
		}
		glfwSwapBuffers(m_window);
	}
}

void Renderer::renderGUI(){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(.0f, .0f));
	ImGui::SetNextWindowSize(ImVec2(m_wWidth / 5, m_wHeight));
	{
		ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		// render your GUI
		if (ImGui::CollapsingHeader("File")){
			if(ImGui::Button("Load Mesh")){
				m_fileDialog.Open();
			}
			if(ImGui::Button("Save Mesh")){
				if(m_mesh){
					m_mesh->writeMesh();
				}
			}
			if(ImGui::Button("Exit")){
				glfwSetWindowShouldClose(m_window, true);
			}
			m_fileDialog.Display();
			if(m_fileDialog.HasSelected()) {
				m_retrieved = false;
				m_normalized = false;
				m_retrieval_text = "";
				m_mesh = MeshMap::Instance()->getMesh(m_fileDialog.GetSelected().string());
				m_mesh->prepare();
				m_camera.setPosition(glm::vec3(0.0f, 0.0f, 1.5f));
				m_fileDialog.ClearSelected();
			}
		}
    	if (ImGui::CollapsingHeader("Mesh")){
			// Display some basic mesh info
			ImGui::TextWrapped((m_mesh) ? m_mesh->getPath().string().c_str() : "Load a mesh!");
			ImGui::Text("# of vertices: %d", (m_mesh) ? m_mesh->countVertices() : 0);
			ImGui::Text("# of faces: %d", (m_mesh) ? m_mesh->countFaces() : 0);
			ImGui::Separator();

			// Create a combo box to allow user to select different draw options
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

			// Add button to reset the position of the camera
			if(ImGui::Button("Reset View")){
				m_mesh->resetTransformations();
				m_camera.setPosition(glm::vec3(0.0f, 0.0f, 1.5f));
			}

			// Allow the user to change the color of the mesh
			ImGui::Text("Mesh Colour Picker");
			ImGui::ColorEdit3("Mesh", &m_meshMaterialDiffuse[0]);

			ImGui::Checkbox("Render Mesh", &m_renderMesh);
			ImGui::Checkbox("Render Convex Hull", &m_renderConvexHull);
			ImGui::Checkbox("Render Unit Cube", &m_renderUnitCube);
		}

		if (ImGui::CollapsingHeader("Remeshing")) {
			if (ImGui::CollapsingHeader("Subdivision")) {
				// Add button for subdivision
				if (ImGui::Button("In Plane")) {
					if (m_mesh) {
						m_mesh->upsample();
					}
				}

				// Add button for loop subdivision (smoothed as it's refined)
				if (ImGui::Button("Loop Subdivision")) {
					if (m_mesh) {
						m_mesh->loopSubdivide();
					}
				}
			}
		}

		if (ImGui::CollapsingHeader("Decimation")) {
			static int target_percentage = 10;
			ImGui::PushItemWidth(100);
			ImGui::SliderInt("Percentage", &target_percentage, 1, 99);
			ImGui::PopItemWidth();

			// Add button for basic decimation
			if (ImGui::Button("Decimate")) {
				if (m_mesh) {
					m_mesh->decimate(m_mesh->countFaces() * 0.01 * target_percentage);
				}
			}

			// Add button for Q-Slim decimation
			if (ImGui::Button("Q-Slim")) {
				if (m_mesh) {
					m_mesh->qslim(m_mesh->countFaces() * 0.01 * target_percentage);
				}
			}
		}

		if (ImGui::CollapsingHeader("Repair")) {

			if (ImGui::Button("Repair Holes")) {
				if (m_mesh) {
					m_mesh->repair();
				}
			}
		}

		if (ImGui::CollapsingHeader("Normalization")) {
			// Add button for scaling
			if (ImGui::Button("Scale to Fit")) {
				if (m_mesh) {
					m_mesh->scale();
				}
			}
			// Add button for centering to view
			if (ImGui::Button("Center To View")) {
				if (m_mesh) {
					m_mesh->centerToView();
				}
			}
			// Add button for alignment
			if (ImGui::Button("Align")) {
				if (m_mesh) {
					m_mesh->alignEigenVectorsToAxes();
				}
			}
			// Add button for flip test
			if (ImGui::Button("Flip Test")) {
				if (m_mesh) {
					m_mesh->flipMirrorTest();
				}
			}
		}

		if (ImGui::CollapsingHeader("3D Descriptors")) {
			ImGui::Text("Surface Area: %f", (m_mesh) ? std::get<float>(m_mesh->getDescriptor(FEAT_AREA_3D)) : 0);
			ImGui::Text("Mesh Volume: %f", (m_mesh) ? std::get<float>(m_mesh->getDescriptor(FEAT_MVOLUME_3D)) : 0);
			ImGui::Text("Bounding Box Volume: %f", (m_mesh) ? std::get<float>(m_mesh->getDescriptor(FEAT_BBVOLUME_3D)) : 0);
			ImGui::Text("Eccentricity: %f", (m_mesh) ? std::get<float>(m_mesh->getDescriptor(FEAT_ECCENTRICITY_3D)) : 0);
			ImGui::Text("Compactness: %f", (m_mesh) ? std::get<float>(m_mesh->getDescriptor(FEAT_COMPACTNESS_3D)) : 0);
			if (ImGui::Button("Compute##Mesh")) {
				if(m_mesh){
					m_mesh->computeFeatures(Descriptors::descriptor_all & ~Descriptors::descriptor_diameter);
				}
			}
		}

		if (ImGui::CollapsingHeader("Convex Hull 3D Descriptors")) {
			ImGui::Text("Surface Area: %f", (m_mesh) ? std::get<float>(m_mesh->getConvexHull()->getDescriptor(FEAT_AREA_3D)) : 0);
			ImGui::Text("Mesh Volume: %f", (m_mesh) ? std::get<float>(m_mesh->getConvexHull()->getDescriptor(FEAT_MVOLUME_3D)) : 0);
			ImGui::Text("Bounding Box Volume: %f", (m_mesh) ? std::get<float>(m_mesh->getConvexHull()->getDescriptor(FEAT_BBVOLUME_3D)) : 0);
			ImGui::Text("Diameter: %f", (m_mesh) ? std::get<float>(m_mesh->getConvexHull()->getDescriptor(FEAT_DIAMETER_3D)) : 0);
			ImGui::Text("Eccentricity: %f", (m_mesh) ? std::get<float>(m_mesh->getConvexHull()->getDescriptor(FEAT_ECCENTRICITY_3D)) : 0);
			ImGui::Text("Compactness: %f", (m_mesh) ? std::get<float>(m_mesh->getConvexHull()->getDescriptor(FEAT_COMPACTNESS_3D)) : 0);
			if (ImGui::Button("Compute##ConvexHull")) {
				if(m_mesh){
					m_mesh->getConvexHull()->computeFeatures();
				}
			}

		}
		// Add button for undo
		if (ImGui::Button("Undo Last Operation")) {
			if (m_mesh) {
				m_mesh->undoLastOperation();
			}
		}

		ImGui::Separator();
		if (ImGui::Button("Choose Shape Database")) {
			m_folderDialog.Open();
		}
		ImGui::SameLine();
		HelpMarker("Choose the root folder of your database");
		ImGui::TextWrapped(m_dbPath.string().c_str());

		m_folderDialog.Display();
		if (m_folderDialog.HasSelected()) {
			m_dbPath = m_folderDialog.GetSelected();
			m_folderDialog.ClearSelected();
			m_featuresPresent = (std::filesystem::exists(m_dbPath/"feats.csv") && std::filesystem::exists(m_dbPath/"feats_avg.csv")) ? true : false;
		}
		if(!m_featuresPresent && !m_dbPath.empty()){
			if(ImGui::Button("Compute DB Features")){
				Stats::getDatabaseFeatures(m_dbPath.string());
				m_featuresPresent = true;
			}
			ImGui::SameLine();
			HelpMarker("Compute the file feats.avg in the DB root");
		}

		if(!m_dbPath.empty()){
			if (ImGui::Button("Generate Screenshots")) {
				takeScreenshots(m_dbPath);
			}
		}

		if (ImGui::CollapsingHeader("Shape Retrieval")) {
			ImGui::Separator();
			if (ImGui::Button("Normalize Shape")) {
				if (m_mesh) {
					m_normalizing_future = std::async(std::launch::async, [&] {
							Mesh mesh_copy = *m_mesh;
							mesh_copy.unprepare();
							mesh_copy.normalize(3000);
							return std::make_shared<Mesh>(mesh_copy);
						}
					);
				}
			}
			ImGui::SameLine();
			HelpMarker("Normalize the shape by getting it to an average of 3000 vertices and by scaling and orientating the mesh");
			if (m_normalizing_future.valid()) {
				auto status = m_normalizing_future.wait_for(std::chrono::seconds(0));
				if (status == std::future_status::timeout) {
					ImGui::Text("Normalizing...");
				}
				else if (status == std::future_status::ready) {
					m_mesh = m_normalizing_future.get();
					m_mesh->prepare();
					m_normalizing_future = std::future<MeshPtr>();
					m_normalized = true;
					ImGui::Text("Normalization Complete!");
				}
			}
			else {
				if (m_normalized) {
					ImGui::Text("Normalization Complete!");
				}
			}
			if(m_featuresPresent){
				ImGui::InputInt("# of Shapes", &m_numShapes);
				if (ImGui::Button("Find Similiar ANN")) {
					if (m_mesh) {
						m_retrieval_future = std::async(std::launch::async, [&] {
							m_retrieval_text = "Searching for the most similar shapes...";
							Retriever::retrieveSimiliarShapesKNN(m_mesh, m_dbPath, m_numShapes);
						});
					}
				}
				ImGui::SameLine();
				HelpMarker("Start searching for shapes using ANN.\nfeats.csv and feats_avg.csv must be present in the DB root");

				if (ImGui::Button("Find Similiar Shapes")) {
					if (m_mesh) {
						m_retrieval_future = std::async(std::launch::async, [&] {
							m_retrieval_text = "Searching for the most similar shapes...";
							Retriever::retrieveSimiliarShapes(m_mesh, m_dbPath);
						});
					}
				}
				ImGui::SameLine();
				HelpMarker("Start searching for shapes.\nfeats.csv and feats_avg.csv must be present in the DB root");

				ImGui::TextWrapped(m_retrieval_text.c_str());

				if (m_retrieval_future.valid()) {
					auto status = m_retrieval_future.wait_for(std::chrono::seconds(0));
					if (status == std::future_status::ready) {
						m_retrieval_future = std::future<void>();
						m_retrieved = true;
						m_retrieval_text = "Search Complete!";
					}
				}
			}
		}

		// Allow user to run DR on the feature vector
		// Plot the results of the DR
		if (ImGui::CollapsingHeader("Dimensionality Reduction")) {
			
			if (ImGui::Button("Run t-SNE")) {
				if (m_dbPath.empty()) {
					ImGui::OpenPopup("Error No Database");
				} else {
					if (m_origFeatureVectors.count() == 0 && m_classTypeToIndicesNames.empty()) {
						FeatureVector::getFeatureVectorClassToIndicesName(m_dbPath, m_origFeatureVectors, m_classTypeToIndicesNames, m_origDimensionality, m_numDataPoints);
					}
					runTSNE(m_origFeatureVectors, m_numDataPoints, m_origDimensionality, m_maxIterations, m_reducedFeatureVectors, m_tsneIterations);
					m_iteration = m_tsneIterations.size();
					m_plotTSE = true;
				}
			}

			if (ImGui::BeginPopup("Error No Database")) {
				ImGui::Text("Load the shape database!");
				ImGui::EndPopup();
			}

			ImGui::InputInt("Number of Iterations", &m_maxIterations);
			ImGui::Checkbox("Visualise Iterations", &m_visualiseIterations);
			ImGui::Checkbox("Animate Iterations", &m_animateIterations);
			ImGui::Checkbox("Display Plot", &m_plotTSE);
			if (m_visualiseIterations) {
				ImGui::SliderInt("Iterations", &m_iteration, 1, m_tsneIterations.size());


				if (!m_tsneIterations.empty()) {
					if (m_iteration > m_tsneIterations.size()) {
						if (m_animateIterations) {
							m_iteration = 1;
						}
						else {
							m_iteration = m_tsneIterations.size();
						}
					}

					const auto tsneIteration = m_tsneIterations[m_iteration - 1];
					Eigen::MatrixXd reducedFeatureVector;
					FeatureVector::formatReducedFeatureVector(tsneIteration, m_numDataPoints, reducedFeatureVector);
					plotTSNE(reducedFeatureVector);
					if (m_animateIterations) m_iteration++;
				}
			} else {
				plotTSNE(m_reducedFeatureVectors);
			}
		} else {
			m_plotTSE = false;
		}
		ImGui::End();
	}

	{
		if (m_retrieved) {
			ImGui::SetNextWindowPos(ImVec2(m_wWidth - (m_wWidth / 3), .0f));
			ImGui::SetNextWindowSize(ImVec2(m_wWidth / 3, m_wHeight));
			ImGui::Begin("Similar Shapes", NULL, ImGuiWindowFlags_NoResize);

			ImGui::Columns(3);
			ImGui::SetColumnWidth(-1, m_wWidth / 9);
			const auto similarShapes = m_mesh->getSimilarShapes();
			for (int i = 0; i < m_numShapes && i < similarShapes.size(); i++) {

				auto filepath = std::filesystem::path(similarShapes.at(i).first);
				loadScreenshot((m_dbPath / filepath).string());
				ImGui::Text("Mesh Name: %s", filepath.filename().string().c_str());
				ImGui::Text("Distance: %f", similarShapes.at(i).second);

				if (meshToTexture.find((m_dbPath / filepath).string()) != meshToTexture.end()) {
					const auto meshTextureInfo = meshToTexture.at((m_dbPath / filepath).string());
					const auto meshTexture = std::get<0>(meshTextureInfo);
					auto textureWidth = std::get<1>(meshTextureInfo);
					auto textureHeight = std::get<2>(meshTextureInfo);
					auto scale = textureWidth / textureHeight;

					if (ImGui::ImageButton((void*)(intptr_t)meshTexture, ImVec2(m_wWidth / 9, (m_wWidth / 9) / scale))) {
						m_retrieved = false;
						m_normalized = false;
						m_retrieval_text = "";
						m_mesh = MeshMap::Instance()->getMesh((m_dbPath / filepath).string());
						m_mesh->prepare();
						m_camera.setPosition(glm::vec3(0.0f, 0.0f, 1.5f));
					}

				} else {
					ImGui::Button("Visualise Shape");
				}
				ImGui::NextColumn();
			}
			ImGui::End();
		}
	}
	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**************** GLFW Callbacks ****************/

void Renderer::resizeWindow(int w, int h){
	m_wWidth = w;
	m_wHeight = h;
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
	if (key == GLFW_KEY_N)
		inputHandler->setKeyValue(KEYBOARD_N, (action == GLFW_PRESS) ? true : false);

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

void Renderer::setupImGuiStyle() {
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

	static ImVector<ImVec4> plotColormap;
	plotColormap.push_back(ImPlot::GetColormapColor(0, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(1, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(2, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(3, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(4, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(5, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(6, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(7, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(8, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(9, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(10, ImPlotColormap_Paired));
	plotColormap.push_back(ImPlot::GetColormapColor(11, ImPlotColormap_Paired));
	ImPlot::AddColormap("Custom", plotColormap.Data, plotColormap.Size, false);
	ImPlot::PushColormap("Custom");
}

void Renderer::takeScreenshots(std::filesystem::path dbPath) {
	for (auto& shape : std::filesystem::recursive_directory_iterator(dbPath)) {
		std::string extension = shape.path().extension().string();
		std::string offExt(".off");
		std::string plyExt(".ply");
		if (extension == offExt || extension == plyExt) {
			m_mesh = MeshMap::Instance()->getMesh(shape.path().string());
			m_mesh->prepare();
			m_camera.setPosition(glm::vec3(0.0f, 0.0f, 1.5f));
			glm::mat4 proj = glm::perspective(m_camera.getFOV(), (float)m_wWidth / (float)m_wHeight, 0.1f, 100.0f);
			glm::mat4 projView = proj * m_camera.getViewMatrix();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, m_wWidth, m_wHeight);

			m_mesh->draw(projView, m_meshMaterialDiffuse, m_camera.getPosition());

			// Make the BYTE array, factor of 3 because it's RBG.
			BYTE* pixels = new BYTE[3 * m_wWidth * m_wHeight];

			glReadPixels(0, 0, m_wWidth, m_wHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

			auto shapePath = shape.path();
			shapePath.replace_extension(".png");

			// Convert to FreeImage format & save to file
			stbi_write_png(shapePath.string().c_str(), m_wWidth, m_wHeight, 3, pixels, m_wWidth * 3);

			delete[] pixels;
			glfwSwapBuffers(m_window);
		}
	}
}

void Renderer::loadScreenshot(std::filesystem::path shapePath) {
	if (meshToTexture.find(shapePath.string()) == meshToTexture.end()) {
		auto pngPath = shapePath;
		pngPath.replace_extension(".png");

		int my_image_width = 0;
		int my_image_height = 0;
		GLuint my_image_texture = 0;
		bool ret = LoadTextureFromFile(pngPath.string().c_str(), &my_image_texture, &my_image_width, &my_image_height);
		if (ret) {
			meshToTexture.emplace(shapePath.string(), std::make_tuple(my_image_texture, my_image_width, my_image_height));
		}
	}
}

void Renderer::runTSNE(Eigen::VectorXd dbFeatureVector, int numOfDataPoints, int origDimensionality, int max_iter, Eigen::MatrixXd& reducedFeatureVectors, std::vector<std::vector<double>>& iterations) {

	// Define some variables
	int outDimensionality = 2;
	double perplexity = 50; 
	double theta = 0.5;
	int rand_seed = -1;
	iterations.clear();

	std::vector<double> outData(numOfDataPoints * outDimensionality);
	// Now fire up the SNE implementation
	TSNE::customRun(&dbFeatureVector[0], numOfDataPoints, origDimensionality, &outData[0], outDimensionality, perplexity, theta, rand_seed, false, max_iter, 250, 250, iterations);
	FeatureVector::formatReducedFeatureVector(outData, numOfDataPoints, reducedFeatureVectors);
}


void Renderer::plotTSNE(Eigen::MatrixXd reducedFeatureVectors) {

	srand(0);
	std::vector<std::tuple<std::string, std::vector<std::string>, std::vector<double>, std::vector<double>>> datasets;
	for (auto& class_it : m_classTypeToIndicesNames) {
		const auto classType = class_it.first;
		const auto indicesNamePairs = class_it.second;
		std::vector<double> xVals;
		std::vector<double> yVals;
		std::vector<std::string> meshNames;
		for (auto& indexName_it : indicesNamePairs) {
			xVals.push_back(reducedFeatureVectors.row(indexName_it.first).x());
			yVals.push_back(reducedFeatureVectors.row(indexName_it.first).y());
			meshNames.push_back(indexName_it.second);
		}
		datasets.push_back(std::make_tuple(classType, meshNames, xVals, yVals));
	}

	if (!datasets.empty() && m_plotTSE) {
		ImGui::SetNextWindowPos(ImVec2(m_wWidth/5.0f, 0));
		ImGui::SetNextWindowSize(ImVec2(4*m_wWidth/5.0f, m_wHeight));
		ImGui::Begin("2D Feature Space", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::Button("Auto Fit")) {
			ImPlot::SetNextAxesToFit();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_plotTSE = false;
		}

		if (ImPlot::BeginPlot("2D Feature Space", ImVec2(-1,-1))) {
			ImPlot::SetupAxis(ImAxis_X1, NULL, ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels);
			ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, 0.1);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 0.1);
			ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Outside | ImPlotLegendFlags_Horizontal);
			std::set<std::string> classes;
			float plotColor = 0.0f;
			for (auto& dataset : datasets) {
				std::string classType = std::get<0>(dataset);
				classes.insert(classType);
				ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.85f);
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 6, ImPlot::SampleColormap(plotColor / (datasets.size() - 1)), IMPLOT_AUTO, ImPlot::SampleColormap(plotColor / (datasets.size() - 1)));
				const auto xVals = std::get<2>(dataset);
				const auto yVals = std::get<3>(dataset);
				ImPlot::PlotScatter(classType.c_str(), &xVals[0], &yVals[0], xVals.size());
				plotColor++;
			}
			if (ImPlot::IsPlotHovered()) {
				// get ImGui window DrawList
				ImPlotPoint mouse = ImPlot::GetPlotMousePos();
				ImVec2 plotSize = ImPlot::GetPlotSize();
				ImPlotRect plotLimits = ImPlot::GetPlotLimits();
				auto xRange = plotLimits.X.Size();
				auto yRange = plotLimits.Y.Size();

				auto xPerPixel = xRange / plotSize.x;
				auto yPerPixel = yRange / plotSize.y;

				// find mouse location index
				for (const auto& dataset : datasets) {
					const auto className = std::get<0>(dataset);
					const auto meshNames = std::get<1>(dataset);
					const auto xVals = std::get<2>(dataset);
					const auto yVals = std::get<3>(dataset);
					bool hoveringOnPoint = false;
					for (int idx = 0; idx < xVals.size(); idx++) {
						if (abs(xVals[idx] - mouse.x) < (xPerPixel * 6)) {
							if (abs(yVals[idx] - mouse.y) < (yPerPixel * 6)) {
								hoveringOnPoint = true;
								ImGui::BeginTooltip();
								ImGui::Text("Mesh Name: %s", meshNames[idx].c_str());
								ImGui::Text("Class Name: %s", className.c_str());
								ImGui::EndTooltip();
								break;
							}
						}
					}

					if (hoveringOnPoint) {
						break;
					}
				}
			}

			ImPlot::EndPlot();
		}

		ImGui::End();
	}
}

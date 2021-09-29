#include "mesh.hpp"
#include "normalization.hpp"
#include "glad/glad.h"
#include "glm/gtx/transform.hpp"
#include "GLFW/glfw3.h"
#include "igl/per_vertex_normals.h"
#include "igl/upsample.h"
#include "igl/loop.h"
#include "igl/decimate.h"
#include "igl/qslim.h"
#include "igl/centroid.h"
#include "input_handler.hpp"
#include <glm/gtx/string_cast.hpp>

Mesh::Mesh(std::filesystem::path path) : meshPath(path){
	if (!Importer::importModel(path, V, F)) {
		return;
	}
	model = glm::mat4(1.0f);
	rotation = glm::vec2(0.0f);
}

Mesh::~Mesh(){
	if(prepared){
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
}

void Mesh::writeMesh(std::filesystem::path filePath) {
	Exporter::exportModel(filePath, V, F);
}

void Mesh::prepare(){
	if(!prepared){
		meshShader.loadShader("shaders/basic_vertex.glsl", GL_VERTEX_SHADER);
		meshShader.loadShader("shaders/basic_fragment.glsl", GL_FRAGMENT_SHADER);
		meshShader.compileShaders();
		edgeShader.loadShader("shaders/basic_vertex.glsl", GL_VERTEX_SHADER);
		edgeShader.loadShader("shaders/edge_fragment.glsl", GL_FRAGMENT_SHADER);
		edgeShader.compileShaders();
	}
	init();
	prepared = true;
}

void Mesh::writeMesh(){
	Exporter::exportModel(meshPath, V, F);
}

void Mesh::upsample(int n){
	saveState();

	igl::upsample(V, F, n);
	recomputeAndRender();
}

void Mesh::normalize(int target){
	const int thresh = 200;
	while(V.rows() < target - thresh || V.rows() > target + thresh){
		while(V.rows() < target - thresh) {
			// Upsample
			upsample();
		} 
		while(V.rows() > target + thresh) { 
			// Downsample
			decimate(F.rows() - F.rows() * 0.01 * 10);
		}
	}
	centerToView();
	alignEigenVectorsToAxes();
	flipMirrorTest();
	scale();
}

void Mesh::loopSubdivide(int n) {
	saveState();

	igl::loop(V, F, V, F, n);
	recomputeAndRender();
}

void Mesh::decimate(int n) {
	saveState();

	Eigen::MatrixXd U;
	Eigen::MatrixXi G;
	Eigen::VectorXi J; 
	if (igl::decimate(V.cast<double>(), F, n, U, G, J)) {
		V = U.cast<float>();
		F = G;
	}
	recomputeAndRender();
}

void Mesh::qslim(int n) {
	saveState();

	Eigen::MatrixXd U;
	Eigen::MatrixXi G;
	Eigen::VectorXi J;
	Eigen::VectorXi I;
	if (igl::qslim(V.cast<double>(), F, n, U, G, J, I)) {
		V = U.cast<float>();
		F = G;
	}
	recomputeAndRender();
}

void Mesh::init() {
	igl::per_vertex_normals(V, F, N);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	dataToOpenGL();
}

void Mesh::dataToOpenGL(){
	std::vector<Vertex> vertices(V.rows());
	for(int i = 0; i < V.rows(); i++){
		vertices[i] = {{V.coeff(i,0),V.coeff(i,1),V.coeff(i,2)},
						{N.coeff(i,0),N.coeff(i,1),N.coeff(i,2)}};
	}
	std::vector<unsigned int> indices(3*F.rows());
	int i=0, j = 0;
	while(i < 3*F.rows()){
		indices[i++] = F.coeff(j, 0);
		indices[i++] = F.coeff(j, 1);
		indices[i++] = F.coeff(j, 2);
		j++;
	}

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex normal
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
}

void Mesh::update(float dt){
	MouseState ms = InputHandler::Instance()->getMouseState();
	if(InputHandler::Instance()->isKeyDown(MOUSE_RIGHT)){
		if(ms.moved){
			rotation.x += 1.0f * ms.dy;
			rotation.y += 1.0f * ms.dx;
		}
	}

	glm::vec4 xAxisViewSpace = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 xAxisViewSpaceToModelSpace = xAxisViewSpace * model;

	glm::vec4 yAxisViewSpace = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 yAxisViewSpaceToModelSpace = yAxisViewSpace * model;

	model = glm::rotate(model, rotation.x * dt, glm::vec3(xAxisViewSpaceToModelSpace.x, xAxisViewSpaceToModelSpace.y, xAxisViewSpaceToModelSpace.z));
	model = glm::rotate(model, rotation.y * dt, glm::vec3(yAxisViewSpaceToModelSpace.x, yAxisViewSpaceToModelSpace.y, yAxisViewSpaceToModelSpace.z));
	rotation = {0.0f, 0.0f};
}

void Mesh::draw(const glm::mat4 &projView, const glm::vec3 &materialDiffuse, const glm::vec3 &cameraPos) {
	glBindVertexArray(VAO);
	unsigned int drawMode = GL_FILL;
	switch(OptionsMap::Instance()->getOption(DRAW_MODE)){
		case WIREFRAME:
			edgeShader.use();
			edgeShader.setUniform("projView", projView);
			edgeShader.setUniform("model", model); 
			drawMode = GL_LINE;
			break;
		case POINT_CLOUD:
			edgeShader.use();
			edgeShader.setUniform("projView", projView);
			edgeShader.setUniform("model", model); 
			drawMode = GL_POINT;
			break;
		case SHADED_MESH:
		default:
			meshShader.use();
			meshShader.setUniform("projView", projView);
			meshShader.setUniform("model", model); 
			meshShader.setUniform("viewPos", cameraPos); 
			meshShader.setUniform("material.diffuse", materialDiffuse);
			meshShader.setUniform("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
			meshShader.setUniform("material.shininess", 32.0f);
			drawMode = GL_FILL;
			break;
	};
	glPolygonMode(GL_FRONT_AND_BACK, drawMode);
	glDrawElements(GL_TRIANGLES, 3*F.rows(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Mesh::resetTransformations(){
	model = glm::mat4(1.0f);
	rotation = glm::vec2(0.0f);
}

void Mesh::scale() {
	saveState();

	Normalization::scale(V);
	recomputeAndRender();
}

void Mesh::centerToView() {
	saveState();

	Eigen::Vector3f c;
	igl::centroid(V, F, c);
	while (c.x() > 0.005f || c.y() > 0.005f || c.z() > 0.005f) {
		for (int i = 0; i < V.rows(); i++) {
			V.row(i) -= c;
		}
		igl::centroid(V, F, c);
	}
	recomputeAndRender();
}

void Mesh::alignEigenVectorsToAxes() {
	saveState();

	Eigen::Vector3f centroid;
	igl::centroid(V, F, centroid);

	const auto covarianceMatrix = Normalization::calculateCovarianceMatrix(V, centroid);
	const auto eigen = Normalization::calculateEigen(covarianceMatrix);
	Normalization::alignPrincipalAxes(V, centroid, eigen[2].first, eigen[1].first);
	recomputeAndRender();
}

void Mesh::flipMirrorTest() {
	saveState();
	Normalization::flipMirrorTest(V, F);
	recomputeAndRender();
}

void Mesh::undoLastOperation() {
	if (backupV.size() != 0 && backupF.size() != 0) {
		V = backupV;
		F = backupF;

		recomputeAndRender();

		backupV = Eigen::MatrixXf{};
		backupF = Eigen::MatrixXi{};
	}
}

void Mesh::saveState() {
	backupV = V;
	backupF = F;
}

void Mesh::recomputeAndRender() {
	if (prepared) {
		igl::per_vertex_normals(V, F, N);
		dataToOpenGL();
	}
}

void Mesh::computeFeatures(){
	Descriptors::computeDescriptors(V, F, Descriptors::descriptor_all, features);
}

float Mesh::getDescriptor(Features f) {
	auto t = features.find(f);
	if(t == features.end()) return 0.0f;
	else return t->second;
}

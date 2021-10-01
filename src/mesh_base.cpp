#include "mesh_base.hpp"
#include "shader_map.hpp"
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

MeshBase::MeshBase() {
	m_modelMatrix = glm::mat4(1.0f);
}

MeshBase::~MeshBase(){
	if(m_prepared){
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}
}

void MeshBase::writeMesh(std::filesystem::path filePath) {
	Exporter::exportModel(filePath, m_vertices, m_faces);
}

void MeshBase::prepare(){
	init();
	m_prepared = true;
}

void MeshBase::upsample(int n){
	saveState();

	igl::upsample(m_vertices, m_faces, n);
	recomputeAndRender();
}

void MeshBase::normalize(int target){
	const int thresh = 200;
	while(m_vertices.rows() < target - thresh || m_vertices.rows() > target + thresh){
		while(m_vertices.rows() < target - thresh) {
			// Upsample
			upsample();
		} 
		while(m_vertices.rows() > target + thresh) {
			// Downsample
			decimate(m_faces.rows() - m_faces.rows() * 0.01 * 10);
		}
	}
	centerToView();
	alignEigenVectorsToAxes();
	flipMirrorTest();
	scale();
}

void MeshBase::loopSubdivide(int n) {
	saveState();

	igl::loop(m_vertices, m_faces, m_vertices, m_faces, n);
	recomputeAndRender();
}

void MeshBase::decimate(int n) {
	saveState();

	Eigen::MatrixXd U;
	Eigen::MatrixXi G;
	Eigen::VectorXi J; 
	if (igl::decimate(m_vertices.cast<double>(), m_faces, n, U, G, J)) {
		m_vertices = U.cast<float>();
		m_faces = G;
	}
	recomputeAndRender();
}

void MeshBase::qslim(int n) {
	saveState();

	Eigen::MatrixXd U;
	Eigen::MatrixXi G;
	Eigen::VectorXi J;
	Eigen::VectorXi I;
	if (igl::qslim(m_vertices.cast<double>(), m_faces, n, U, G, J, I)) {
		m_vertices = U.cast<float>();
		m_faces = G;
	}
	recomputeAndRender();
}

void MeshBase::init() {
	igl::per_vertex_normals(m_vertices, m_faces, m_normals);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	dataToOpenGL();
}

void MeshBase::dataToOpenGL(){
	std::vector<Vertex> vertices(m_vertices.rows());
	for(int i = 0; i < m_vertices.rows(); i++){
		vertices[i] = {{m_vertices.coeff(i,0),m_vertices.coeff(i,1),m_vertices.coeff(i,2)},
						{m_normals.coeff(i,0),m_normals.coeff(i,1),m_normals.coeff(i,2)}};
	}
	std::vector<unsigned int> indices(3*m_faces.rows());
	int i=0, j = 0;
	while(i < 3*m_faces.rows()){
		indices[i++] = m_faces.coeff(j, 0);
		indices[i++] = m_faces.coeff(j, 1);
		indices[i++] = m_faces.coeff(j, 2);
		j++;
	}

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex normal
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
}

void MeshBase::update(float dt){
	glm::vec2 rotation = { 0, 0 };
	MouseState ms = InputHandler::Instance()->getMouseState();
	if(InputHandler::Instance()->isKeyDown(MOUSE_RIGHT)){
		if(ms.moved){
			rotation.x += 1.0f * ms.dy;
			rotation.y += 1.0f * ms.dx;
		}
	}

	glm::vec4 xAxisViewSpace = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 xAxisViewSpaceToModelSpace = xAxisViewSpace * m_modelMatrix;

	glm::vec4 yAxisViewSpace = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 yAxisViewSpaceToModelSpace = yAxisViewSpace * m_modelMatrix;

	m_modelMatrix = glm::rotate(m_modelMatrix, rotation.x * dt, glm::vec3(xAxisViewSpaceToModelSpace.x, xAxisViewSpaceToModelSpace.y, xAxisViewSpaceToModelSpace.z));
	m_modelMatrix = glm::rotate(m_modelMatrix, rotation.y * dt, glm::vec3(yAxisViewSpaceToModelSpace.x, yAxisViewSpaceToModelSpace.y, yAxisViewSpaceToModelSpace.z));
}

void MeshBase::draw(const glm::mat4 &projView, const glm::vec3 &materialDiffuse, const glm::vec3 &cameraPos) {
	glBindVertexArray(m_VAO);
	unsigned int drawMode = GL_FILL;
	switch(OptionsMap::Instance()->getOption(DRAW_MODE)){
		case WIREFRAME:
			ShaderMap::Instance()->getShader(SHADER_EDGE)->use();
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("projView", projView);
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("model", m_modelMatrix);
			drawMode = GL_LINE;
			break;
		case POINT_CLOUD:
			ShaderMap::Instance()->getShader(SHADER_EDGE)->use();
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("projView", projView);
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("model", m_modelMatrix);
			drawMode = GL_POINT;
			break;
		case SHADED_MESH:
		default:
			ShaderMap::Instance()->getShader(SHADER_BASE)->use();
			ShaderMap::Instance()->getShader(SHADER_BASE)->setUniform("projView", projView);
			ShaderMap::Instance()->getShader(SHADER_BASE)->setUniform("model", m_modelMatrix);
			ShaderMap::Instance()->getShader(SHADER_BASE)->setUniform("viewPos", cameraPos); 
			ShaderMap::Instance()->getShader(SHADER_BASE)->setUniform("material.diffuse", materialDiffuse);
			ShaderMap::Instance()->getShader(SHADER_BASE)->setUniform("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
			ShaderMap::Instance()->getShader(SHADER_BASE)->setUniform("material.shininess", 32.0f);
			drawMode = GL_FILL;
			break;
	};
	glPolygonMode(GL_FRONT_AND_BACK, drawMode);
	glDrawElements(GL_TRIANGLES, 3 * m_faces.rows(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void MeshBase::resetTransformations(){
	m_modelMatrix = glm::mat4(1.0f);
}

void MeshBase::scale() {
	saveState();

	Normalization::scale(m_vertices);
	recomputeAndRender();
}

void MeshBase::alignEigenVectorsToAxes() {
	saveState();

	Eigen::Vector3f centroid;
	igl::centroid(m_vertices, m_faces, centroid);

	const auto covarianceMatrix = Normalization::calculateCovarianceMatrix(m_vertices, centroid);
	const auto eigen = Normalization::calculateEigen(covarianceMatrix);
	Normalization::alignPrincipalAxes(m_vertices, centroid, eigen[2].first, eigen[1].first);
	recomputeAndRender();
}

void MeshBase::flipMirrorTest() {
	saveState();
	Normalization::flipMirrorTest(m_vertices, m_faces);
	recomputeAndRender();
}

void MeshBase::undoLastOperation() {

	if (m_backupV.size() != 0 && m_backupF.size() != 0) {
		m_vertices = m_backupV;
		m_faces = m_backupF;

		recomputeAndRender();

		m_backupV = Eigen::MatrixXf{};
		m_backupF = Eigen::MatrixXi{};
	}
}

void MeshBase::saveState() {
	m_backupV = m_vertices;
	m_backupF = m_faces;
}

void MeshBase::recomputeAndRender() {
	if (m_prepared) {
		igl::per_vertex_normals(m_vertices, m_faces, m_normals);
		dataToOpenGL();
	}
}
void MeshBase::computeFeatures(){
	Descriptors::computeDescriptors(m_vertices, m_faces, Descriptors::descriptor_all, features);
}

float MeshBase::getDescriptor(Features f) {
	auto t = features.find(f);
	if(t == features.end()) return 0.0f;
	else return t->second;
}

void MeshBase::getCentroid(Eigen::Vector3f &c){
	igl::centroid(m_vertices, m_faces, c);
}
void MeshBase::centerToView() {
	saveState();
	Eigen::Vector3f c;
	getCentroid(c);
	for (int i = 0; i < m_vertices.rows(); i++) {
		m_vertices.row(i) -= c;
	}
	recomputeAndRender();
}

#include "mesh.hpp"
#include "glad/glad.h"
#include "glm/gtx/transform.hpp"
#include "GLFW/glfw3.h"
#include "input_handler.hpp"

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices) : Mesh(vertices, indices, "shaders/basic_vertex.glsl", "shaders/basic_fragment.glsl") {}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::string vShader, std::string fShader) : vertices(std::move(vertices)), indices(std::move(indices)), currentShader{meshShader} {
	meshShader.loadShader(vShader.c_str(), GL_VERTEX_SHADER);
	meshShader.loadShader(fShader.c_str(), GL_FRAGMENT_SHADER);
	meshShader.compileShaders();
	edgeShader.loadShader(vShader.c_str(), GL_VERTEX_SHADER);
	edgeShader.loadShader("shaders/edge_fragment.glsl", GL_FRAGMENT_SHADER);
	edgeShader.compileShaders();
	init();
}

Mesh::Mesh(std::string path) : Mesh(path, "shaders/basic_vertex.glsl", "shaders/basic_fragment.glsl") {}

Mesh::Mesh(std::string path, std::string vShader, std::string fShader) : currentShader{meshShader} {
	Loader::loadModel(path, vertices, indices, nFaces);
	meshShader.loadShader(vShader.c_str(), GL_VERTEX_SHADER);
	meshShader.loadShader(fShader.c_str(), GL_FRAGMENT_SHADER);
	meshShader.compileShaders();
	edgeShader.loadShader(vShader.c_str(), GL_VERTEX_SHADER);
	edgeShader.loadShader("shaders/edge_fragment.glsl", GL_FRAGMENT_SHADER);
	edgeShader.compileShaders();
	init();
}

Mesh::~Mesh(){
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Mesh::init() {
	model = glm::mat4(1.0f);
	rotation = glm::vec2(0.0f);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

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

void Mesh::mouseMoved(int dx, int dy){
}

void Mesh::update(float dt){
	model = glm::mat4(1.0f);
	MouseState ms = InputHandler::Instance()->getMouseState();
	if(InputHandler::Instance()->isKeyDown(MOUSE_RIGHT)){
		if(ms.moved){
			rotation.x += 2.0f * ms.dy;
			rotation.y += 2.0f * ms.dx;
		}
	}
	model = glm::rotate(model, rotation.x * dt, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, rotation.y * dt, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Mesh::draw(glm::mat4 projView) {
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
			meshShader.setUniform("material.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
			drawMode = GL_FILL;
			break;
	};
	glPolygonMode(GL_FRONT_AND_BACK, drawMode);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

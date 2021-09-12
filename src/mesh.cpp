#include "mesh.hpp"
#include "glad/glad.h"
#include "glm/gtx/transform.hpp"
#include "GLFW/glfw3.h"
#include "input_handler.hpp"

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices) : Mesh(vertices, indices, "shaders/basic_vertex.glsl", "shaders/basic_fragment.glsl") {}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::string vShader, std::string fShader) : vertices(std::move(vertices)), indices(std::move(indices)) {
	shader.loadShader(vShader.c_str(), GL_VERTEX_SHADER);
	shader.loadShader(fShader.c_str(), GL_FRAGMENT_SHADER);
	shader.compileShaders();
	init();
}

Mesh::Mesh(std::string path) : Mesh(path, "shaders/basic_vertex.glsl", "shaders/basic_fragment.glsl") {}

Mesh::Mesh(std::string path, std::string vShader, std::string fShader) {
	Loader::loadOFF(path, vertices, indices);
	shader.loadShader(vShader.c_str(), GL_VERTEX_SHADER);
	shader.loadShader(fShader.c_str(), GL_FRAGMENT_SHADER);
	shader.compileShaders();
	init();
}

Mesh::~Mesh(){
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Mesh::init() {
	model = glm::mat4(1.0f);
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

void Mesh::update(float dt){
	if(InputHandler::Instance()->getMouseState().rightDown)
		model = glm::rotate(model, (float)glm::radians(10.0f) * 10.0f * dt, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Mesh::draw(glm::mat4 projView) {
	shader.use();
	shader.setUniform("projView", projView);
	shader.setUniform("model", model); 
	shader.setUniform("color", glm::vec3(0.8f, 0.8f, 0.8f));
	
	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	
	if(OptionsMap::Instance()->getOption(DRAW_EDGES)){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader.setUniform("color", glm::vec3(0.1f, 0.1f, 0.1f));
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}

	glBindVertexArray(0);
}

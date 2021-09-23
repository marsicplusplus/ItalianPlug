#include "unit_cube.hpp"
#include "igl/per_vertex_normals.h"

UnitCube::UnitCube() {
	shader.loadShader("shaders/cube_vertex.glsl", GL_VERTEX_SHADER);
	shader.loadShader("shaders/edge_fragment.glsl", GL_FRAGMENT_SHADER);
	shader.compileShaders();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

UnitCube::~UnitCube() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void UnitCube::draw(const glm::mat4& projView) {
	glBindVertexArray(VAO);
	shader.use();
	shader.setUniform("projView", projView);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

#include "unit_cube.h"
#include "igl/per_vertex_normals.h"

UnitCube::UnitCube() {
	meshShader.loadShader("shaders/basic_vertex.glsl", GL_VERTEX_SHADER);
	meshShader.loadShader("shaders/basic_fragment.glsl", GL_FRAGMENT_SHADER);
	meshShader.compileShaders();
	edgeShader.loadShader("shaders/basic_vertex.glsl", GL_VERTEX_SHADER);
	edgeShader.loadShader("shaders/edge_fragment.glsl", GL_FRAGMENT_SHADER);
	edgeShader.compileShaders();
	model = glm::mat4(1.0f);
	rotation = glm::vec2(0.0f);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	V.conservativeResize(8, 3);
	F.conservativeResize(12, 3);

	for (int i = 0; i < 8; i++) {
		V.row(i) = Eigen::Vector3f(cube_vertices[i * 3], cube_vertices[i * 3 + 1], cube_vertices[i * 3 + 2]);
	}

	for (int i = 0; i < 12; i++) {
		F.row(i) = Eigen::Vector3i(cube_elements[i * 3], cube_elements[i * 3 + 1], cube_elements[i * 3 + 2]);
	}

	igl::per_vertex_normals(V, F, N);

	std::vector<Vertex> vertices(V.rows());
	for (int i = 0; i < V.rows(); i++) {
		vertices[i] = { {V.coeff(i,0),V.coeff(i,1),V.coeff(i,2)},
						{N.coeff(i,0),N.coeff(i,1),N.coeff(i,2)} };
	}
	std::vector<unsigned int> indices(3 * F.rows());
	int i = 0, j = 0;
	while (i < 3 * F.rows()) {
		indices[i++] = F.coeff(j, 0);
		indices[i++] = F.coeff(j, 1);
		indices[i++] = F.coeff(j, 2);
		j++;
	}

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
}

UnitCube::~UnitCube() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void UnitCube::draw(const glm::mat4& projView, const glm::vec3& materialDiffuse, const glm::vec3& cameraPos) {
	glBindVertexArray(VAO);
	unsigned int drawMode = GL_FILL;
	switch (OptionsMap::Instance()->getOption(DRAW_MODE)) {
	case POINT_CLOUD:
		edgeShader.use();
		edgeShader.setUniform("projView", projView);
		edgeShader.setUniform("model", model);
		drawMode = GL_POINT;
		break;
	case WIREFRAME:
	case SHADED_MESH:
	default:
		edgeShader.use();
		edgeShader.setUniform("projView", projView);
		edgeShader.setUniform("model", model);
		drawMode = GL_LINE;
		break;
	};

	glPolygonMode(GL_FRONT_AND_BACK, drawMode);
	glDrawElements(GL_TRIANGLES, 3 * F.rows(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}
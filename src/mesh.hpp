#ifndef __MESH_HPP__
#define __MESH_HPP__

#include "utils.hpp"
#include <vector>
#include "glm/mat4x4.hpp"
#include "shader.hpp"

class Mesh {
	public:
		Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices);
		Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::string vShader, std::string fShader);
		Mesh(std::string path);
		Mesh(std::string path, std::string vShader, std::string fShader);

		~Mesh();

		void draw(glm::mat4 projView);
		void update(float dt);

	private:
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		glm::mat4 model;
		Shader shader;

		void init();
};

#endif

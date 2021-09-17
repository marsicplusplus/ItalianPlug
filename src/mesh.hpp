#ifndef __MESH_HPP__
#define __MESH_HPP__

#include "utils.hpp"
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "shader.hpp"

class Mesh {
	public:
		Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices);
		Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::string vShader, std::string fShader);
		Mesh(std::string path);
		Mesh(std::string path, std::string vShader, std::string fShader);

		~Mesh();

		void draw(glm::mat4 projView, glm::vec3 materialDiffuse);
		void update(float dt);
		void mouseMoved(int dx, int dy);

		Shader &currentShader;

	private:
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int nFaces;

		Shader meshShader;
		Shader edgeShader;
		glm::mat4 model;
		glm::vec2 rotation;

		void init();
};

#endif

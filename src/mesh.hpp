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

		inline int countVertices() const {return vertices.size();}
		inline int countFaces() const {return indices.size() / 3;}
		inline std::string getPath() const {return path;}

		void draw(glm::mat4 projView);
		void update(float dt);
		void resetTransformations();

	private:
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::string path;

		Shader meshShader;
		Shader edgeShader;
		glm::mat4 model;
		glm::vec2 rotation;

		void init();
		glm::vec3 calcBarycenter();
};

#endif

#ifndef __MESH_HPP__
#define __MESH_HPP__

#include "utils.hpp"
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "shader.hpp"
#include "igl/readOFF.h"
#include "Eigen/Dense"

class Mesh {
	public:
		Mesh(std::string path);
		Mesh(std::string path, std::string vShader, std::string fShader);

		~Mesh();

		inline int countVertices() const {return V.rows();}
		inline int countFaces() const {return F.rows();}
		inline std::string getPath() const {return path;}

		void draw(const glm::mat4 &projView, const glm::vec3 &matterialDiffuse, const glm::vec3 &cameraPos);
		void update(float dt);
		void resetTransformations();

	private:
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;

		Eigen::MatrixXf V;
		Eigen::MatrixXi F;
		std::string path;

		Shader meshShader;
		Shader edgeShader;
		glm::mat4 model;
		glm::vec2 rotation;

		void init();
		glm::vec3 calcBarycenter();
};

#endif

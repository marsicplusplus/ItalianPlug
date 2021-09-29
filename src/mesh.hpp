#ifndef __MESH_HPP__
#define __MESH_HPP__

#include "utils.hpp"
#include "descriptors.hpp"
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "shader.hpp"
#include "Eigen/Dense"

class Mesh {
	public:
		Mesh(std::filesystem::path path);
		Mesh(std::filesystem::path path, std::string vShader, std::string fShader);

		~Mesh();

		inline int countVertices() const {return V.rows();}
		inline int countFaces() const {return F.rows();}
		inline std::filesystem::path getPath() const { return meshPath; }

		void draw(const glm::mat4 &projView, const glm::vec3 &matterialDiffuse, const glm::vec3 &cameraPos);
		void update(float dt);
		void resetTransformations();
		void writeMesh();
		void writeMesh(std::filesystem::path filePath);
		void normalize(int targetVerts);
		void prepare();
		void computeFeatures();
		float getDescriptor(Features f);

		// Subdivision
		void upsample(int n = 1);
		void loopSubdivide(int n = 1);

		// Decimation
		void decimate(int n = 3000);
		void qslim(int n = 3000);

		// Normalization
		void scale();
		void centerToView();
		void alignEigenVectorsToAxes();
		void flipMirrorTest();
		void undoLastOperation();

	private:
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;

		Eigen::MatrixXf V;
		Eigen::MatrixXi F;
		Eigen::MatrixXf N;

		Eigen::MatrixXf backupV;
		Eigen::MatrixXi backupF;

		std::filesystem::path meshPath;

		Shader meshShader;
		Shader edgeShader;
		glm::mat4 model;
		glm::vec2 rotation;
		bool prepared = false;

		std::unordered_map<Features, float> features;

		void init();
		void dataToOpenGL();
		void saveState();
		void recomputeAndRender();
};

#endif

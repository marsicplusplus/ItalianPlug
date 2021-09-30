#pragma once

#include "utils.hpp"
#include "descriptors.hpp"
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "shader.hpp"
#include "Eigen/Dense"

class MeshBase {
	public:
		MeshBase();
		virtual ~MeshBase();

		inline int countVertices() const {return V.rows();}
		inline int countFaces() const {return F.rows();}
		inline std::shared_ptr<Descriptors> getDescriptors() const {return descriptors;}

		void draw(const glm::mat4 &projView, const glm::vec3 &matterialDiffuse, const glm::vec3 &cameraPos);
		void update(float dt);
		void resetTransformations();
		void writeMesh(std::filesystem::path filePath);
		void normalize(int targetVerts);
		void prepare();

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

	protected:
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;

		Eigen::MatrixXf V;
		Eigen::MatrixXi F;
		Eigen::MatrixXf N;

		Eigen::MatrixXf backupV;
		Eigen::MatrixXi backupF;

		Shader meshShader;
		Shader edgeShader;
		glm::mat4 model;
		glm::vec2 rotation;
		bool prepared = false;

		std::shared_ptr<Descriptors> descriptors;

		void init();
		void dataToOpenGL();
		void saveState();
		void recomputeAndRender();
};
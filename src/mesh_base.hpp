#ifndef __MESH_BASE_HPP__
#define __MESH_BASE_HPP__

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

		inline int countVertices() const {return m_vertices.rows();}
		inline int countFaces() const {return m_faces.rows();}

		inline Eigen::MatrixXf getVertices() const { return m_vertices; }
		inline Eigen::MatrixXi getFaces() const { return m_faces; }

		void writeMesh(std::filesystem::path filePath);
		void normalize(int targetVerts);

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

		float getDescriptor(Features f);
		void compute3DFeatures(unsigned int desc = Descriptors::descriptor3d_all);
		void compute2DFeatures(unsigned int desc = Descriptors::descriptor2d_all);

		void getCentroid(Eigen::Vector3f &c);

		virtual void recomputeAndRender();
		virtual void draw(const glm::mat4& projView, const glm::vec3& matterialDiffuse, const glm::vec3& cameraPos);
		virtual void update(float dt);
		virtual void prepare();
		virtual void resetTransformations();
		void renderSilhouette(uint8_t *fb);

	protected:
		unsigned int m_VAO;
		unsigned int m_VBO;
		unsigned int m_EBO;

		Eigen::MatrixXf m_vertices;
		Eigen::MatrixXi m_faces;
		Eigen::MatrixXf m_normals;

		Eigen::MatrixXf m_backupV;
		Eigen::MatrixXi m_backupF;

		Shader m_meshShader;
		Shader m_edgeShader;

		glm::mat4 m_modelMatrix;
		bool m_prepared = false;

		std::unordered_map<Features, float> features;

		void init();
		void dataToOpenGL();
		void saveState();
};

#endif

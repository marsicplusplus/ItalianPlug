#pragma once 

#include "Eigen/Dense"

class Descriptors {
	public:
		Descriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		~Descriptors();

		float getArea() { return m_area; }
		float getMeshVolume() { return m_meshVolume; }
		float getBoundingBoxVolume() { return m_boundingBoxVolume; }
		float getDiameter() { return m_diameter; }
		float getCompactness() { return m_compactness; }
		float getEccentricity() { return m_eccentricity; }

		void computeDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags);
		void computeArea(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeMeshVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeBoundingBoxVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeCompactness();
		void computeEccentricity(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeDiameter(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);

		float computeAngle3RandomVertices(const Eigen::MatrixXf& V);
		float distanceBetweenTwoPoints(const Eigen::Vector3f& pointA, const Eigen::Vector3f& pointB);
		float distanceBetween2RandomVeritces(const Eigen::MatrixXf& V);
		float distanceBetweenBarycenterAndRandomVertex(const Eigen::MatrixXf& V, const Eigen::Vector3f& centroid);
		float sqrtAreaOfTriange3RandomVertices(const Eigen::MatrixXf& V);
		float cubeRootVolumeTetrahedron4RandomVertices(const Eigen::MatrixXf& V);

		enum descriptors3D : unsigned int
		{
			descriptor_area					= 1 << 0,
			descriptor_meshVolume			= 1 << 1,
			descriptor_boundingBoxVolume	= 1 << 2,
			descriptor_diameter				= 1 << 3,
			descriptor_compactness			= 1 << 4,
			descriptor_eccentricity			= 1 << 5,
			descriptor_all					= 255 
		};

	private:
		float m_area;
		float m_meshVolume;
		float m_boundingBoxVolume;
		float m_diameter;
		float m_compactness;
		float m_eccentricity;
};


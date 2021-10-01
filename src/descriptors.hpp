#ifndef __DESCRIPTORS_HPP__
#define __DESCRIPTORS_HPP__

#include "Eigen/Dense"
#include "utils.hpp"


namespace Descriptors {
		void computeDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags, std::unordered_map<Features, float> &feats);
		float computeArea(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeMeshVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeBoundingBoxVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeCompactness(float m_area, float m_meshVolume);
		float computeEccentricity(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeDiameter(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);

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
};

#endif

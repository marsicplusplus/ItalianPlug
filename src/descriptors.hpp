#ifndef __DESCRIPTORS_HPP__
#define __DESCRIPTORS_HPP__

#include "Eigen/Dense"
#include "utils.hpp"


namespace Descriptors {
		void compute3DDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags, std::unordered_map<Features, float> &feats);
		void compute2DDescriptors(const uint8_t *fb, unsigned int flags, std::unordered_map<Features, float> &feats);
		int compute2DArea(const uint8_t* fb);
		int compute2DPerimeter(const uint8_t* fb);
		float compute2DCompactness(int area, int perimeter);

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
			descriptor3d_area					= 1 << 0,
			descriptor3d_meshVolume				= 1 << 1,
			descriptor3d_boundingBoxVolume		= 1 << 2,
			descriptor3d_diameter				= 1 << 3,
			descriptor3d_compactness			= 1 << 4,
			descriptor3d_eccentricity			= 1 << 5,
			descriptor3d_all					= 255 
		};

		enum descriptors2D : unsigned int {
			descriptor2d_area					= 1 << 0,
			descriptor2d_perimeter				= 1 << 1,
			descriptor2d_compactness			= 1 << 2,
			descriptor2d_all					= 255
		};
};

#endif

#pragma once 

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


#ifndef __DESCRIPTORS_HPP__
#define __DESCRIPTORS_HPP__

#include "Eigen/Dense"
#include "utils.hpp"
#include "histogram.hpp"
#include <variant>
#include <vector>
#include <map>

typedef std::variant<int, float, Histogram> DescriptorType;

namespace Descriptors {
		void computeDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags, std::unordered_map<Features, DescriptorType> &feats);
		float computeArea(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeMeshVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeBoundingBoxVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeCompactness(float m_area, float m_meshVolume);
		float computeEccentricity(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		float computeDiameter(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);

		std::vector<float> computeAngle3RandomVertices(const Eigen::MatrixXf& Vertices, int numberOfSamples);
		std::vector<float> distanceBetween2RandomVeritces(const Eigen::MatrixXf& Vertices, int numberOfSamples);
		std::vector<float> distanceBetweenBarycenterAndRandomVertex(const Eigen::MatrixXf& V, const Eigen::Vector3f& centroid, int numberOfSamples);
		std::vector<float> sqrtAreaOfTriange3RandomVertices(const Eigen::MatrixXf& Vertices, int numberOfSamples);
		std::vector<float> cubeRootVolumeTetrahedron4RandomVertices(const Eigen::MatrixXf& Vertices, int numberOfSamples);


		Histogram computeA3Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins);
		Histogram computeD1Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins);
		Histogram computeD2Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins);
		Histogram computeD3Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins);
		Histogram computeD4Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins);

		float distanceBetweenTwoPoints(const Eigen::Vector3f& pointA, const Eigen::Vector3f& pointB);

		enum descriptors3D : uint16_t
		{
			descriptor_area					= 1 << 0,
			descriptor_meshVolume			= 1 << 1,
			descriptor_boundingBoxVolume	= 1 << 2,
			descriptor_diameter				= 1 << 3,
			descriptor_compactness			= 1 << 4,
			descriptor_eccentricity			= 1 << 5,
			descriptor_a3					= 1 << 6,
			descriptor_d1					= 1 << 7,
			descriptor_d2					= 1 << 8,
			descriptor_d3					= 1 << 9,
			descriptor_d4					= 1 << 9,
			descriptor_all					= ~0x0000 & 0xFFFF 
		};
};

#endif

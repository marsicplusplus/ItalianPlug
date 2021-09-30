#define _USE_MATH_DEFINES

#include "descriptors.hpp"
#include "normalization.hpp"
#include "igl/centroid.h"
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4996)

void Descriptors::computeDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags, std::unordered_map<Features, float> &feats) {
	if (flags & descriptor_area) feats[FEAT_AREA_3D] = computeArea(V, F);
	if (flags & descriptor_meshVolume) feats[FEAT_MVOLUME_3D] = computeMeshVolume(V, F);
	if (flags & descriptor_boundingBoxVolume) feats[FEAT_BBVOLUME_3D] = computeBoundingBoxVolume(V, F);
	if (flags & descriptor_compactness) feats[FEAT_COMPACTNESS_3D] = computeCompactness(feats[FEAT_AREA_3D], feats[FEAT_MVOLUME_3D]);
	if (flags & descriptor_eccentricity) feats[FEAT_ECCENTRICITY_3D] = computeEccentricity(V, F);
	if (flags & descriptor_diameter) feats[FEAT_DIAMETER_3D] = computeDiameter(V, F);
}

float Descriptors::computeArea(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
	auto meshArea = 0.0f;
	for (int f = 0; f < F.rows(); f++) {

		auto point1 = V.row(F(f, 0));
		auto point2 = V.row(F(f, 1));
		auto point3 = V.row(F(f, 2));

		Eigen::Vector3f vecA = point1 - point2;
		Eigen::Vector3f vecB = point1 - point3;
		float triangleArea = vecA.cross(vecB).norm() / 2;
		meshArea += triangleArea;
	}
	return meshArea;
}

float Descriptors::computeMeshVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {

	auto meshVolume = 0.0f;
	for (int f = 0; f < F.rows(); f++) {

		Eigen::Vector3f point1 = V.row(F(f, 0));
		Eigen::Vector3f point2 = V.row(F(f, 1));
		Eigen::Vector3f point3 = V.row(F(f, 2));

		meshVolume += (point1.cross(point2)).dot(point3);
	}

	return std::abs(meshVolume) / 6;
}

float Descriptors::computeBoundingBoxVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
	Eigen::Vector3f min = V.colwise().minCoeff();
	Eigen::Vector3f max = V.colwise().maxCoeff();

	Eigen::Vector3f diff = max - min;
	diff = diff.array().abs();

	return diff.x() * diff.y() * diff.z();
}

float Descriptors::computeCompactness(float m_area, float m_meshVolume) {
	return pow(m_area, 3) / (36 * M_PI * m_meshVolume * m_meshVolume);
}

float Descriptors::computeEccentricity(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
	Eigen::Vector3f centroid;
	igl::centroid(V, F, centroid);
	const auto covarianceMatrix = Normalization::calculateCovarianceMatrix(V, centroid);
	const auto eigen = Normalization::calculateEigen(covarianceMatrix);
	return std::abs(eigen[2].second) / std::abs(eigen[0].second);
}

float Descriptors::computeDiameter(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {

	// TO DO! 
	Eigen::Vector3f min = V.colwise().minCoeff();
	Eigen::Vector3f max = V.colwise().maxCoeff();

	Eigen::Vector3f diff = max - min;
	diff = diff.array().abs();

	return diff.maxCoeff();
}

#pragma warning( pop )

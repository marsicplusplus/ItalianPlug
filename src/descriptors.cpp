#define _USE_MATH_DEFINES

#include "descriptors.hpp"
#include "normalization.hpp"
#include "igl/centroid.h"
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4996)

Descriptors::Descriptors(){}

Descriptors::~Descriptors(){}

void Descriptors::compute3DDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags) {
	if (flags & descriptor_area) computeArea(V, F);
	if (flags & descriptor_meshVolume) computeMeshVolume(V, F);
	if (flags & descriptor_boundingBoxVolume) computeBoundingBoxVolume(V, F);
	if (flags & descriptor_compactness) computeCompactness();
	if (flags & descriptor_eccentricity) computeEccentricity(V, F);
	if (flags & descriptor_diameter) computeDiameter(V, F);
}

void Descriptors::compute2DDescriptors(uint8_t* fb, int wW, int wH, unsigned int flags) {
	if (flags & descriptor_2Darea) compute2DArea(fb, wW, wH);
}

void Descriptors::computeArea(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
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
	m_area = meshArea;
}

void Descriptors::computeMeshVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {

	auto meshVolume = 0.0f;
	for (int f = 0; f < F.rows(); f++) {

		Eigen::Vector3f point1 = V.row(F(f, 0));
		Eigen::Vector3f point2 = V.row(F(f, 1));
		Eigen::Vector3f point3 = V.row(F(f, 2));

		meshVolume += (point1.cross(point2)).dot(point3);
	}

	m_meshVolume = std::abs(meshVolume) / 6;
}


void Descriptors::compute2DArea(uint8_t* fb, int wW, int wH) {
	m_2dArea = 0;
	int k = 0;
	for(int i = 0; i < wW; i++){
		for(int j = 0; j < wH; j++){
			if(fb[k] == 255 && fb[k+1] == 255 && fb[k+2]){
				m_2dArea++;
			}
			k += 3;
		}
	}
}

void Descriptors::computeBoundingBoxVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
	Eigen::Vector3f min = V.colwise().minCoeff();
	Eigen::Vector3f max = V.colwise().maxCoeff();

	Eigen::Vector3f diff = max - min;
	diff = diff.array().abs();

	m_boundingBoxVolume = diff.x() * diff.y() * diff.z();
}

void Descriptors::computeCompactness() {
	m_compactness = pow(m_area, 3) / (36 * M_PI * m_meshVolume * m_meshVolume);
}

void Descriptors::computeEccentricity(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
	Eigen::Vector3f centroid;
	igl::centroid(V, F, centroid);
	const auto covarianceMatrix = Normalization::calculateCovarianceMatrix(V, centroid);
	const auto eigen = Normalization::calculateEigen(covarianceMatrix);
	m_eccentricity = std::abs(eigen[2].second) / std::abs(eigen[0].second);
}

void Descriptors::computeDiameter(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {

	// TO DO! 
	Eigen::Vector3f min = V.colwise().minCoeff();
	Eigen::Vector3f max = V.colwise().maxCoeff();

	Eigen::Vector3f diff = max - min;
	diff = diff.array().abs();

	m_diameter = diff.maxCoeff();
}

#pragma warning( pop )

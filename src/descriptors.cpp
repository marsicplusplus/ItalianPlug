#define _USE_MATH_DEFINES

#include "descriptors.hpp"
#include "normalization.hpp"
#include "igl/centroid.h"
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4996)

Descriptors::Descriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F){
	computeDescriptors(V, F, descriptor_all);
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

	auto max_distance = 0.0f;
	// Naive Solution N^2
	// This should be computed on the convex hull of a mesh, not the mesh itself!
	for (int i = 0; i < V.rows(); i++) {

		const auto pointA = V.row(i);
		for (int j = 0; j < V.rows(); j++) {
			const auto pointB = V.row(j);
			const auto diff = pointA - pointB;
			const auto distance = diff.norm();
			if (distance > max_distance) {
				max_distance = distance;
			}
		}
	}

	return max_distance;
}

float Descriptors::computeAngle3RandomVertices(const Eigen::MatrixXf& V) {

	// -->    -->   --->   --->
	// AB dot BC = ||AB|| ||BC|| cos(theta)
	//
	//                 -->    -->
    //               ( AB dot BC )
    // theta = arccos( --------- )
	//               (  --->   ---> )}
	//               || AB|| ||BC || )

	const auto v1 = rand() % V.rows();
	const auto v2 = rand() % V.rows();
	const auto v3 = rand() % V.rows();

	const Eigen::Vector3f pointA = V.row(v1);
	const Eigen::Vector3f pointB = V.row(v2);
	const Eigen::Vector3f pointC = V.row(v3);

	const Eigen::Vector3f AB = pointB - pointA;
	const Eigen::Vector3f BC = pointC - pointB;

	const auto dotProduct = AB.dot(BC);
	const auto theta = acos(dotProduct / (AB.norm() * BC.norm()));

	return theta;
}

float Descriptors::distanceBetweenTwoPoints(const Eigen::Vector3f& pointA, const Eigen::Vector3f& pointB) {
	// d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2) ^ 1/2 
	return sqrt(pow(pointB.x() - pointA.x(), 2) + pow(pointB.y() - pointA.y(), 2) + pow(pointB.z() - pointA.z(), 2));
}

float Descriptors::distanceBetween2RandomVeritces(const Eigen::MatrixXf& V) {
	const auto v1 = rand() % V.rows();
	const auto v2 = rand() % V.rows();

	const Eigen::Vector3f pointA = V.row(v1);
	const Eigen::Vector3f pointB = V.row(v2);
	return distanceBetweenTwoPoints(pointA, pointB);
}

float Descriptors::distanceBetweenBarycenterAndRandomVertex(const Eigen::MatrixXf& V, const Eigen::Vector3f& centroid) {
	const auto v1 = rand() % V.rows();
	const Eigen::Vector3f pointA = V.row(v1);

	return distanceBetweenTwoPoints(centroid, pointA);
}

float Descriptors::sqrtAreaOfTriange3RandomVertices(const Eigen::MatrixXf& V) {
	const auto v1 = rand() % V.rows();
	const auto v2 = rand() % V.rows();
	const auto v3 = rand() % V.rows();

	const Eigen::Vector3f pointA = V.row(v1);
	const Eigen::Vector3f pointB = V.row(v2);
	const Eigen::Vector3f pointC = V.row(v3);

	Eigen::Vector3f BA = pointA - pointB;
	Eigen::Vector3f CA = pointA - pointC;
	float triangleArea = (BA.cross(CA)).norm() / 2;

	return sqrt(triangleArea);
}

float Descriptors::cubeRootVolumeTetrahedron4RandomVertices(const Eigen::MatrixXf& V) {

	//  V=1/6|(a×b)⋅c|

	const auto v1 = rand() % V.rows();
	const auto v2 = rand() % V.rows();
	const auto v3 = rand() % V.rows();
	const auto v4 = rand() % V.rows();

	const Eigen::Vector3f pointA = V.row(v1);
	const Eigen::Vector3f pointB = V.row(v2);
	const Eigen::Vector3f pointC = V.row(v3);
	const Eigen::Vector3f pointD = V.row(v3);

	return abs((pointA - pointD).dot((pointB - pointD).cross(pointC - pointD))) / 6;
}


#pragma warning( pop )

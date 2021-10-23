#define _USE_MATH_DEFINES

#include "descriptors.hpp"
#include "normalization.hpp"
#include "igl/centroid.h"
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4996)

void Descriptors::computeDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags, DescriptorMap &feats) {
	if (flags & descriptor_area) feats[FEAT_AREA_3D] = computeArea(V, F);
	if (flags & descriptor_meshVolume) feats[FEAT_MVOLUME_3D] = computeMeshVolume(V, F);
	if (flags & descriptor_boundingBoxVolume) feats[FEAT_BBVOLUME_3D] = computeBoundingBoxVolume(V, F);
	if (flags & descriptor_compactness) feats[FEAT_COMPACTNESS_3D] = computeCompactness(std::get<float>(feats[FEAT_AREA_3D]), std::get<float>(feats[FEAT_MVOLUME_3D]));
	if (flags & descriptor_eccentricity) feats[FEAT_ECCENTRICITY_3D] = computeEccentricity(V, F);
	if (flags & descriptor_diameter) feats[FEAT_DIAMETER_3D] = computeDiameter(V, F);
	if (flags & descriptor_a3) feats[FEAT_A3_3D] = computeA3Histogram(V, F, 10);
	if (flags & descriptor_d1) feats[FEAT_D1_3D] = computeD1Histogram(V, F, 10);
	if (flags & descriptor_d2) feats[FEAT_D2_3D] = computeD2Histogram(V, F, 10);
	if (flags & descriptor_d3) feats[FEAT_D3_3D] = computeD3Histogram(V, F, 10);
	if (flags & descriptor_d4) feats[FEAT_D4_3D] = computeD4Histogram(V, F, 10);
}

Histogram Descriptors::computeD1Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins) {
	const int nSamples = 500000;
	Eigen::Vector3f centroid;
	igl::centroid(V, F, centroid);
	std::vector<float> values = distanceBetweenBarycenterAndRandomVertex(V, centroid, nSamples);

	std::sort(values.begin(), values.end());
	float min = values[0];
	float max = values[values.size() - 1];
	for(auto &a : values){
		a = (float)(a - min)/(float)(max - min);
	}
	
	Histogram histogram(bins);
	histogram.populate(values);
	histogram.normalize();
	return histogram;
}

Histogram Descriptors::computeD2Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins) {
	const int nSamples = 500000;
	std::vector<float> values = distanceBetween2RandomVeritces(V, nSamples);

	std::sort(values.begin(), values.end());
	float min = values[0];
	float max = values[values.size() - 1];
	for(auto &a : values){
		a = (float)(a - min)/(float)(max - min);
	}

	Histogram histogram(bins);
	histogram.populate(values);
	histogram.normalize();
	return histogram;
}

Histogram Descriptors::computeD3Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins) {
	const int nSamples = 500000;
	std::vector<float> values = sqrtAreaOfTriange3RandomVertices(V, nSamples);

	std::sort(values.begin(), values.end());
	float min = values[0];
	float max = values[values.size() - 1];
	for(auto &a : values){
		a = (float)(a - min)/(float)(max - min);
	}
	
	Histogram histogram(bins);
	histogram.populate(values);
	histogram.normalize();
	return histogram;
}

Histogram Descriptors::computeD4Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins) {
	const int nSamples = 500000;
	std::vector<float> values = cubeRootVolumeTetrahedron4RandomVertices(V, nSamples);

	std::sort(values.begin(), values.end());
	float min = values[0];
	float max = values[values.size() - 1];
	for(auto &a : values){
		a = (float)(a - min)/(float)(max - min);
	}

	Histogram histogram(bins);
	histogram.populate(values);
	histogram.normalize();
	return histogram;
}

Histogram Descriptors::computeA3Histogram(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, int bins) {
	const int nSamples = 500000;
	std::vector<float> values = computeAngle3RandomVertices(V, nSamples);
	std::sort(values.begin(), values.end());
	float min = values[0];
	float max = values[values.size() - 1];
	for(auto &a : values){
		a = (float)(a - min)/(float)(max - min);
	}

	Histogram histogram(bins);
	histogram.populate(values);
	histogram.normalize();
	return histogram;
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

float Descriptors::computeCompactness(float area, float volume) {
	return pow(area, 3) / (36 * M_PI * volume * volume);
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

std::vector<float> Descriptors::computeAngle3RandomVertices(const Eigen::MatrixXf& Vertices, int numberOfSamples) {

	// -->    -->   --->   --->
	// AB dot BC = ||AB|| ||BC|| cos(theta)
	//
	//                 -->    -->
    //               ( AB dot BC )
    // theta = arccos( --------- )
	//               (  --->   ---> )}
	//               || AB|| ||BC || )

	std::vector<float> angleResults;
	const auto cubeRootOfSamples = std::cbrt(numberOfSamples);

	for (int firstLoop = 0; firstLoop < cubeRootOfSamples; firstLoop++) {
		const auto v1 = rand() % Vertices.rows();
		const Eigen::Vector3f pointA = Vertices.row(v1);

		for (int secondLoop = 0; secondLoop < cubeRootOfSamples; secondLoop++) {
			auto v2 = rand() % Vertices.rows();

			while (v2 == v1) {
				v2 = rand() % Vertices.rows();
			}

			const Eigen::Vector3f pointB = Vertices.row(v2);

			for (int thirdLoop = 0; thirdLoop < cubeRootOfSamples; thirdLoop++) {
				auto v3 = rand() % Vertices.rows();

				while (v3 == v1 || v3 == v2) {
					v3 = rand() % Vertices.rows();
				}

				const Eigen::Vector3f pointC = Vertices.row(v3);

				const Eigen::Vector3f AB = pointB - pointA;
				const Eigen::Vector3f BC = pointC - pointB;

				const auto dotProduct = AB.dot(BC);
				const auto theta = acos(dotProduct / (AB.norm() * BC.norm()));
				angleResults.push_back(theta);
			}
		}
	}

	return angleResults;
}

float Descriptors::distanceBetweenTwoPoints(const Eigen::Vector3f& pointA, const Eigen::Vector3f& pointB) {
	// d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2) ^ 1/2 
	return sqrt(pow(pointB.x() - pointA.x(), 2) + pow(pointB.y() - pointA.y(), 2) + pow(pointB.z() - pointA.z(), 2));
}

std::vector<float> Descriptors::distanceBetween2RandomVeritces(const Eigen::MatrixXf& Vertices, int numberOfSamples) {

	std::vector<float> distanceResults;
	const auto squareRootOfSamples = std::sqrt(numberOfSamples);

	for (int firstLoop = 0; firstLoop < squareRootOfSamples; firstLoop++) {
		const auto v1 = rand() % Vertices.rows();
		const Eigen::Vector3f pointA = Vertices.row(v1);

		for (int secondLoop = 0; secondLoop < squareRootOfSamples; secondLoop++) {
			auto v2 = rand() % Vertices.rows();

			while (v2 == v1) {
				v2 = rand() % Vertices.rows();
			}

			const Eigen::Vector3f pointB = Vertices.row(v2);
			distanceResults.push_back(distanceBetweenTwoPoints(pointA, pointB));
		}
	}

	return distanceResults;
}

std::vector<float> Descriptors::distanceBetweenBarycenterAndRandomVertex(const Eigen::MatrixXf& V, const Eigen::Vector3f& centroid, int numberOfSamples) {

	std::vector<float> distanceResults;
	for (int i = 0; i < numberOfSamples; i++) {
		const auto v1 = rand() % V.rows();
		const Eigen::Vector3f pointA = V.row(v1);

		distanceResults.push_back(distanceBetweenTwoPoints(centroid, pointA));
	}

	return distanceResults;
}

std::vector<float> Descriptors::sqrtAreaOfTriange3RandomVertices(const Eigen::MatrixXf& Vertices, int numberOfSamples) {

	std::vector<float> areaResults;
	const auto cubeRootOfSamples = std::cbrt(numberOfSamples);

	for (int firstLoop = 0; firstLoop < cubeRootOfSamples; firstLoop++) {
		const auto v1 = rand() % Vertices.rows();
		const Eigen::Vector3f pointA = Vertices.row(v1);

		for (int secondLoop = 0; secondLoop < cubeRootOfSamples; secondLoop++) {
			auto v2 = rand() % Vertices.rows();

			while (v2 == v1) {
				v2 = rand() % Vertices.rows();
			}

			const Eigen::Vector3f pointB = Vertices.row(v2);

			for (int thirdLoop = 0; thirdLoop < cubeRootOfSamples; thirdLoop++) {
				auto v3 = rand() % Vertices.rows();

				while (v3 == v1 || v3 == v2) {
					v3 = rand() % Vertices.rows();
				}

				const Eigen::Vector3f pointC = Vertices.row(v3);

				Eigen::Vector3f BA = pointA - pointB;
				Eigen::Vector3f CA = pointA - pointC;
				float triangleArea = (BA.cross(CA)).norm() / 2;
				areaResults.push_back(sqrt(triangleArea));
			}
		}
	}

	return areaResults;
}

std::vector<float> Descriptors::cubeRootVolumeTetrahedron4RandomVertices(const Eigen::MatrixXf& Vertices, int numberOfSamples) {

	//  V=1/6|(a×b)⋅c|
	std::vector<float> volumeResults;
	const auto fourthRootOfSamples = std::pow(numberOfSamples, 1.0 / 4);
	for (int firstLoop = 0; firstLoop < fourthRootOfSamples; firstLoop++) {
		const auto v1 = rand() % Vertices.rows();
		const Eigen::Vector3f pointA = Vertices.row(v1);

		for (int secondLoop = 0; secondLoop < fourthRootOfSamples; secondLoop++) {
			auto v2 = rand() % Vertices.rows();

			while (v2 == v1) {
				v2 = rand() % Vertices.rows();
			}

			const Eigen::Vector3f pointB = Vertices.row(v2);

			for (int thirdLoop = 0; thirdLoop < fourthRootOfSamples; thirdLoop++) {
				auto v3 = rand() % Vertices.rows();

				while (v3 == v1 || v3 == v2) {
					v3 = rand() % Vertices.rows();
				}

				const Eigen::Vector3f pointC = Vertices.row(v3);

				for (int fourthLoop = 0; fourthLoop < fourthRootOfSamples; fourthLoop++) {
					auto v4 = rand() % Vertices.rows();

					while (v4 == v1 || v4 == v2 || v4 == v3) {
						v4 = rand() % Vertices.rows();
					}

					const Eigen::Vector3f pointD = Vertices.row(v4);

					auto cubeRootVolume = abs((pointA - pointD).dot((pointB - pointD).cross(pointC - pointD))) / 6;
					volumeResults.push_back(cubeRootVolume);
				}
			}
		}
	}

	return volumeResults;
}

#pragma warning( pop )

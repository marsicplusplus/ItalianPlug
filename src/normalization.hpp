#pragma once

#include "glad/glad.h"
#include "glm/vec3.hpp"
#include <any>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <Eigen/Core>
#include "Eigen/Dense"

namespace Normalization {

	template <typename T> int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	void scale(Eigen::MatrixXf& V);

	Eigen::Matrix3f calculateCovarianceMatrix(Eigen::MatrixXf& V, const Eigen::Vector3f& centroid);

	std::vector<Eigen::Vector3f> calculateEigenVectors(const Eigen::Matrix3f& covarianceMatrix);

	void flipMirrorTest(Eigen::MatrixXf& V, const Eigen::MatrixXi& F);

	void alignPrincipalAxes(Eigen::MatrixXf& V, const Eigen::Vector3f& centroid, const Eigen::Vector3f& majorEigenVector, const Eigen::Vector3f& minorEigenVector);

};

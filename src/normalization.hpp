#ifndef __NORMALIZATION_HPP__
#define __NORMALIZATION_HPP__

#include <Eigen/Core>
#include "Eigen/Dense"

namespace Normalization {

	template <typename T> int sgn(T val) {
		auto res =  (T(0) < val) - (val < T(0));
		if (res == 0) {
			res = 1;
		}

		return res;
	}

	void scale(Eigen::MatrixXf& V);

	Eigen::Matrix3f calculateCovarianceMatrix(const Eigen::MatrixXf& V, const Eigen::Vector3f& centroid);

	std::vector<std::pair<Eigen::Vector3f, float>> calculateEigen(const Eigen::Matrix3f& covarianceMatrix);

	void flipMirrorTest(Eigen::MatrixXf& V, const Eigen::MatrixXi& F);

	void alignPrincipalAxes(Eigen::MatrixXf& V, const Eigen::Vector3f& centroid, const Eigen::Vector3f& majorEigenVector, const Eigen::Vector3f& minorEigenVector);

};

#endif
